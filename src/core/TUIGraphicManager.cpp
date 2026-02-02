#include "../include/TUIGraphicManager.hpp"

/* Credit:
    Getting terminal height and weight : https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c

    Used in TUIGraphicManager().
*/

namespace TUI {

TUIGraphicManager::TUIGraphicManager() : TUIheight_(), TUIWidth_(), colorator_(), command{""} {
    winsize ws{};
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) throw std::logic_error("Error couldn't init TUIGraphicManager");
    TUIheight_ = ws.ws_row;
    TUIWidth_ = ws.ws_col;
}

void TUIGraphicManager::TUIAction(const std::string_view& string_action) {
    write(STDOUT_FILENO, string_action.data(), string_action.size());
};

void TUIGraphicManager::update() {
    winsize ws{};
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) throw std::logic_error("Error couldn't init TUIGraphicManager");
    TUIheight_ = ws.ws_row;
    TUIWidth_ = ws.ws_col;
}

void TUIGraphicManager::TUICursorHome() { TUIAction(ActionTable::string_CursorHome); }

void TUIGraphicManager::TUIClear() { TUIAction(ActionTable::string_Clear); }

void TUIGraphicManager::TUIClearCusorToRight() { TUIAction(ActionTable::string_ClearCusorToRight); }

void TUIGraphicManager::TUIClearCusorToLeft() { TUIAction(ActionTable::string_ClearCusorToLeft); }

void TUIGraphicManager::TUIClearCursorLine() {TUIAction(ActionTable::string_ClearCusorLine);}

void TUIGraphicManager::TUIHideCursor() { TUIAction(ActionTable::string_HideCursor); }

void TUIGraphicManager::TUIShowCursor() { TUIAction(ActionTable::string_ShowCursor); }

void TUIGraphicManager::TUIResetColors() { TUIAction(ActionTable::string_ResetColors); }

void TUIGraphicManager::TUIEnterAltScreen() { TUIAction(ActionTable::string_EnterAltScreen); }

void TUIGraphicManager::TUILeaveAltScreen() { TUIAction(ActionTable::string_LeaveAltScreen); }

void TUIGraphicManager::TUISetColor(RGBPanel colorFront, RGBPanel colorBack) {
        colorator_.SetRGBColor(colorFront, colorBack);
        colorator_.ApplyColor();
};

void TUIGraphicManager::TUIDisplayChar(const std::string_view& glyph) {
    write(STDOUT_FILENO, glyph.data(), glyph.size());
}

void TUIGraphicManager::TUImoveCursor(int height, int width) {
    std::string moveCursorOrder{std::format(ActionTable::string_MoveCursor, height, width)};
    write(STDOUT_FILENO, moveCursorOrder.data(), moveCursorOrder.size());
}

void TUIGraphicManager::enableRawMode() {
    tcgetattr(STDIN_FILENO, &original_termios);

    termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void TUIGraphicManager::disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void TUIGraphicManager::addPixel(const std::string_view& glyph, RGBPanel colorFront, RGBPanel colorBack) {
    colorator_.SetRGBColor(colorFront, colorBack);
    command.append(colorator_.GetColorString());
    command.append(std::string(glyph));
}

void TUIGraphicManager::addPixel(const std::string_view& glyph) {
    command.append(std::string(glyph));
}

void TUIGraphicManager::execCommand() { 
    TUIAction(command); 
    command = "";
};

}// namespace
#include "../include/TUIGraphicManager.hpp"

namespace TUI {

TUIGraphicManager::TUIGraphicManager() : TUIheight_(), TUIWidth_(), colorator_() {
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

void TUIGraphicManager::TUISetColor(RGBPanel colorFront, RGBPanel colorBack) {
        colorator_.SetRGBColor(colorFront, colorBack);
        colorator_.ApplyColor();
};

void TUIGraphicManager::TUIDisplayChar(const char chr) { std::cout << chr; }

void TUIGraphicManager::TUImoveCursor(size_t height, size_t width) {
    std::string moveCursorOrder{std::format(ActionTable::string_MoveCursor, height, width)};
    write(STDOUT_FILENO, moveCursorOrder.data(), moveCursorOrder.size());
}

}// namespace
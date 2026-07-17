#include "../include/TUIGraphicManager.hpp"

#include <cerrno>
#include <stdexcept>

namespace TUI {

TUIGraphicManager::TUIGraphicManager() {
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        throw std::logic_error("Error couldn't init TUIGraphicManager");
    }

    TUIheight_ = ws.ws_row;
    TUIWidth_ = ws.ws_col;

    // Avoid repeated reallocations while building terminal frames.
    reserveCommand(static_cast<std::size_t>(TUIheight_) * static_cast<std::size_t>(TUIWidth_) * 24U);
}

void TUIGraphicManager::TUIAction(std::string_view stringAction) {
    const char* data{stringAction.data()};
    std::size_t remaining{stringAction.size()};

    while (remaining > 0U) {
        const ssize_t written{write(STDOUT_FILENO, data, remaining)};

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error("TUIGraphicManager: terminal write failed");
        }

        data += written;
        remaining -= static_cast<std::size_t>(written);
    }
}

void TUIGraphicManager::update() {
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        throw std::logic_error("Error couldn't update TUIGraphicManager");
    }

    TUIheight_ = ws.ws_row;
    TUIWidth_ = ws.ws_col;
}

void TUIGraphicManager::TUICursorHome() { TUIAction(ActionTable::string_CursorHome); }
void TUIGraphicManager::TUIClear() { TUIAction(ActionTable::string_Clear); }
void TUIGraphicManager::TUIClearCusorToRight() { TUIAction(ActionTable::string_ClearCusorToRight); }
void TUIGraphicManager::TUIClearCusorToLeft() { TUIAction(ActionTable::string_ClearCusorToLeft); }
void TUIGraphicManager::TUIClearCursorLine() { TUIAction(ActionTable::string_ClearCusorLine); }
void TUIGraphicManager::TUIHideCursor() { TUIAction(ActionTable::string_HideCursor); }
void TUIGraphicManager::TUIShowCursor() { TUIAction(ActionTable::string_ShowCursor); }
void TUIGraphicManager::TUIResetColors() { TUIAction(ActionTable::string_ResetColors); }
void TUIGraphicManager::TUIEnterAltScreen() { TUIAction(ActionTable::string_EnterAltScreen); }
void TUIGraphicManager::TUILeaveAltScreen() { TUIAction(ActionTable::string_LeaveAltScreen); }

void TUIGraphicManager::TUISetColor(RGBPanel colorFront, RGBPanel colorBack) {
    colorator_.SetRGBColor(colorFront, colorBack);
    colorator_.ApplyColor();
}

void TUIGraphicManager::TUIDisplayChar(std::string_view glyph) {
    TUIAction(glyph);
}

void TUIGraphicManager::TUImoveCursor(int height, int width) {
    const std::string moveCursorOrder{
        std::format(ActionTable::string_MoveCursor, height, width)
    };
    TUIAction(moveCursorOrder);
}

void TUIGraphicManager::enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        throw std::runtime_error("TUIGraphicManager: tcgetattr failed");
    }

    termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        throw std::runtime_error("TUIGraphicManager: tcsetattr failed");
    }
}

void TUIGraphicManager::disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void TUIGraphicManager::reserveCommand(std::size_t capacity) {
    command.reserve(capacity);
}

void TUIGraphicManager::beginFrame() {
    command.clear();
    command.append(ActionTable::string_BeginSynchronizedUpdate);
}

void TUIGraphicManager::endFrame() {
    command.append(ActionTable::string_EndSynchronizedUpdate);
}

void TUIGraphicManager::addCursorHome() {
    command.append(ActionTable::string_CursorHome);
}

void TUIGraphicManager::addCursorMove(int height, int width) {
    command.append(std::format(ActionTable::string_MoveCursor, height, width));
}

void TUIGraphicManager::addPixel(std::string_view glyph, RGBPanel colorFront, RGBPanel colorBack) {
    colorator_.SetRGBColor(colorFront, colorBack);
    command.append(colorator_.GetColorString());
    command.append(glyph);
}

void TUIGraphicManager::addPixel(std::string_view glyph) {
    command.append(glyph);
}

void TUIGraphicManager::execCommand() {
    TUIAction(command);
    command.clear();
}

} // namespace TUI

#include "../include/TUIGraphicManager.hpp"

namespace tui {

TUIGraphicManager::TUIGraphicManager() : TUIheight_(), TUIWidth_() {
    winsize ws{};
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) throw std::logic_error("Error couldn't init TUIGraphicManager");
    TUIheight_ = ws.ws_row;
    TUIWidth_ = ws.ws_col;
}

void TUIGraphicManager::TUIAction(const char* string_action) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), string_action);
    write(STDOUT_FILENO, buf, len);
};


void TUIGraphicManager::TUICursorHome() { TUIAction(ActionTable::string_CursorHome); }

void TUIGraphicManager::TUIClear() { TUIAction(ActionTable::string_Clear); }

void TUIGraphicManager::TUIClearCusorToRight() { TUIAction(ActionTable::string_ClearCusorToRight); }

void TUIGraphicManager::TUIClearCusorToLeft() { TUIAction(ActionTable::string_ClearCusorToLeft); }

void TUIGraphicManager::TUIClearCursorLine() {TUIAction(ActionTable::string_ClearCusorLine);}

void TUIGraphicManager::TUIHideCursor() { TUIAction(ActionTable::string_HideCursor); }

void TUIGraphicManager::TUIShowCursor() { TUIAction(ActionTable::string_ShowCursor); }

void TUIGraphicManager::TUIResetColors() { TUIAction(ActionTable::string_ResetColors); }

void TUIGraphicManager::TUIDisplayChar(const char chr) { std::cout << chr; }

void TUIGraphicManager::TUImoveCursor(int height, int width) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), ActionTable::string_MoveCursor, height, width);
    write(STDOUT_FILENO, buf, len);
}

}// namespace
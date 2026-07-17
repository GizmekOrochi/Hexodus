#ifndef TUIGRAPHICMANAGER_HPP
#define TUIGRAPHICMANAGER_HPP

#pragma once

#include <cstddef>
#include <format>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "TUI-Coloring/Colorator.hpp"
#include "TUIStruct.hpp"

namespace TUI {

class TUIGraphicManager {
private:
    int TUIheight_{};
    int TUIWidth_{};
    Colorator colorator_{};
    termios original_termios{};
    std::string command{};

    void TUIAction(std::string_view stringAction);

public:
    [[nodiscard]] int getHeigth() const noexcept { return TUIheight_; }
    [[nodiscard]] int getWidth() const noexcept { return TUIWidth_; }

    TUIGraphicManager();
    void update();

    void TUIEnterAltScreen();
    void TUILeaveAltScreen();

    void TUICursorHome();
    void TUImoveCursor(int height, int width);

    void TUIClear();
    void TUIClearCusorToRight();
    void TUIClearCusorToLeft();
    void TUIClearCursorLine();

    void TUIHideCursor();
    void TUIShowCursor();

    void TUIResetColors();
    void TUISetColor(RGBPanel colorFront, RGBPanel colorBack);

    void TUIDisplayChar(std::string_view glyph);

    void enableRawMode();
    void disableRawMode();

    // Buffered terminal command construction.
    void reserveCommand(std::size_t capacity);
    void beginFrame();
    void endFrame();
    void addCursorHome();
    void addCursorMove(int height, int width);
    void addPixel(std::string_view glyph, RGBPanel colorFront, RGBPanel colorBack);
    void addPixel(std::string_view glyph);
    void execCommand();
};

} // namespace TUI

#endif // TUIGRAPHICMANAGER_HPP

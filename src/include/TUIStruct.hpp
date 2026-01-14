#ifndef TUISRUCT_HPP
#define TUISRUCT_HPP

#pragma once
#include <string_view>

namespace tui {

enum TUI16Color {
    black = 0,
    red = 1,
    green = 2,
    yellow = 3,
    blue = 4,
    magenta = 5,
    cyan = 6,
    white = 7,
};

namespace ActionTable {
    //Moving
    constexpr std::string_view string_CursorHome{"\x1b[H"};
    constexpr std::string_view string_MoveCursor{"\x1b[{};{}H"};
    //Clearing
    constexpr std::string_view string_Clear{"\x1b[2J"};
    constexpr std::string_view string_ClearCusorToRight{"\x1b[0K"};
    constexpr std::string_view string_ClearCusorToLeft{"\x1b[1K"};
    constexpr std::string_view string_ClearCusorLine{"\x1b[2K"};
    //Cusor things
    constexpr std::string_view string_HideCursor{"\x1b[?25l"};
    constexpr std::string_view string_ShowCursor{"\x1b[?25h"};
    //Coloring
    constexpr std::string_view string_ResetColors{"\x1b[0m"};
    constexpr std::string_view string_16ColorFGMode{"ESC[3{}m"};
    constexpr std::string_view string_16ColorBGMode{"ESC[9{}m"};
};

} //namespace


#endif
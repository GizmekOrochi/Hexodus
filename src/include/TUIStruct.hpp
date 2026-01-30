#ifndef TUISRUCT_HPP
#define TUISRUCT_HPP

#pragma once
#include <string_view>

namespace TUI {

enum TUIcolorModAvalible {
    _none,
    _16Colors,
    _256Colors,
    _trueColor
};

// Used in the 16 color mode
enum TUI16Color {
    black = 0,
    red = 1,
    green = 2,
    yellow = 3,
    blue = 4,
    magenta = 5,
    cyan = 6,
    white = 7,
    // Bright colors (standard 16-color terminal palette)
    bright_black = 8,
    bright_red = 9,
    bright_green = 10,
    bright_yellow = 11,
    bright_blue = 12,
    bright_magenta = 13,
    bright_cyan = 14,
    bright_white = 15
};

// represent a color
struct RGBPanel {
    unsigned char R_, G_, B_;
};

// take a color and turn it into one of the 16 color in the enum
inline TUI16Color transfor16ColorMod(const RGBPanel& RGBColor) {
    unsigned char r = RGBColor.R_ > 128 ? 255 : 0;
    unsigned char g = RGBColor.G_ > 128 ? 255 : 0;
    unsigned char b = RGBColor.B_ > 128 ? 255 : 0;

    bool isBright = (RGBColor.R_ > 192 && RGBColor.G_ > 192 && RGBColor.B_ > 192);
    
    if (r == 0 && g == 0 && b == 0) return isBright ? bright_black : black;
    else if (r == 255 && g == 0 && b == 0) return isBright ? bright_red : red;
    else if (r == 0 && g == 255 && b == 0) return isBright ? bright_green : green;
    else if (r == 255 && g == 255 && b == 0) return isBright ? bright_yellow : yellow;
    else if (r == 0 && g == 0 && b == 255) return isBright ? bright_blue : blue;
    else if (r == 255 && g == 0 && b == 255) return isBright ? bright_magenta : magenta;
    else if (r == 0 && g == 255 && b == 255) return isBright ? bright_cyan : cyan;
    else if (r == 255 && g == 255 && b == 255) return isBright ? bright_white : white;
    return black;
}

// Strings using to ORDER the terminal to OBEY as WE want
namespace ActionTable {
    // Moving
    constexpr std::string_view string_CursorHome{"\x1b[H"};
    constexpr std::string_view string_MoveCursor{"\x1b[{};{}H"};

    // Clearing
    constexpr std::string_view string_Clear{"\x1b[2J"};
    constexpr std::string_view string_ClearCusorToRight{"\x1b[0K"};
    constexpr std::string_view string_ClearCusorToLeft{"\x1b[1K"};
    constexpr std::string_view string_ClearCusorLine{"\x1b[2K"};

    // Cursor
    constexpr std::string_view string_HideCursor{"\x1b[?25l"};
    constexpr std::string_view string_ShowCursor{"\x1b[?25h"};

    // Screen buffer
    constexpr std::string_view string_EnterAltScreen{"\x1b[?1049h"};
    constexpr std::string_view string_LeaveAltScreen{"\x1b[?1049l"};

    // Coloring
    constexpr std::string_view string_ResetColors{"\x1b[0m"};
    constexpr std::string_view string_16ColorFGMod{"\x1b[3{}m"};
    constexpr std::string_view string_16ColorBGMod{"\x1b[4{}m"};
    constexpr std::string_view string_trueColorFGMod{"\x1b[38;2;{};{};{}m"};
    constexpr std::string_view string_trueColorBGMod{"\x1b[48;2;{};{};{}m"};
}


} //namespace


#endif
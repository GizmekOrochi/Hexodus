#include "../../include/TUI-Coloring/Colorator.hpp"

namespace TUI {

/* Credit:

    How does unix coloring work : https://unix.stackexchange.com/questions/148/colorizing-your-terminal-and-shell-environment 
                                  https://stackoverflow.com/questions/10003136/what-is-the-difference-between-xterm-color-xterm-256color
                                  ( And even more tiny post there and there ... this part was REALLY annoying )
        Explanation of how the thing TERM work used everywhere in the Colorator

*/

Colorator::Colorator() : RGBFront_(), RGBBack_(), ColorMod_() {
    bool hasTrueColor = false;
    bool has256Color = false;
    if (const char* colorterm = getenv("COLORTERM")) {
        if (strstr(colorterm, "truecolor") || strstr(colorterm, "24bit")) {
            hasTrueColor = true;
        }
    }
    if (const char* term = getenv("TERM")) {
        if (strstr(term, "256color") || strstr(term, "xterm-256color")) {
            has256Color = true;
        }
    }
    if (hasTrueColor) {
        ColorMod_ = TUIcolorModAvalible::_trueColor;
    } else if (has256Color) {
        ColorMod_ = TUIcolorModAvalible::_256Colors;
    } else {
        ColorMod_ = TUIcolorModAvalible::_16Colors;
    }
}

RGBPanel Colorator::getRGBFront() { return RGBFront_; }

RGBPanel Colorator::getRGBBack() { return RGBBack_; }

void Colorator::SetRGBColor(RGBPanel colorFront, RGBPanel colorBack) {
    RGBFront_ = colorFront;
    RGBBack_ = colorBack;
}

void Colorator::ApplyColor() {
    switch (ColorMod_) {
    case TUIcolorModAvalible::_16Colors: {
        std::string colorFront{std::format(ActionTable::string_16ColorFGMod, static_cast<int>(transfor16ColorMod(RGBFront_)))};
        write(STDOUT_FILENO, colorFront.data(), colorFront.size());
        std::string colorBack{std::format(ActionTable::string_16ColorBGMod, static_cast<int>(transfor16ColorMod(RGBBack_)))};
        write(STDOUT_FILENO, colorBack.data(), colorBack.size());
        break;
    }

    case TUIcolorModAvalible::_256Colors: { // not suported yet 16 color mod logic
        std::string colorFront{std::format(ActionTable::string_16ColorFGMod, static_cast<int>(transfor16ColorMod(RGBFront_)))};
        write(STDOUT_FILENO, colorFront.data(), colorFront.size());
        std::string colorBack{std::format(ActionTable::string_16ColorBGMod, static_cast<int>(transfor16ColorMod(RGBBack_)))};
        write(STDOUT_FILENO, colorBack.data(), colorBack.size());
        break;
    }

    case TUIcolorModAvalible::_trueColor: {
        std::string colorFront{std::format(ActionTable::string_trueColorFGMod, static_cast<int>(RGBFront_.R_), static_cast<int>(RGBFront_.G_), static_cast<int>(RGBFront_.B_))};
        write(STDOUT_FILENO, colorFront.data(), colorFront.size());
        std::string colorBack{std::format(ActionTable::string_trueColorBGMod, static_cast<int>(RGBBack_.R_), static_cast<int>(RGBBack_.G_), static_cast<int>(RGBBack_.B_))};
        write(STDOUT_FILENO, colorBack.data(), colorBack.size());
        break;
    }
    default:
    
        break;
    }
}

std::string Colorator::GetColorString() const {
    std::string result;
    
    switch (ColorMod_) {
    case TUIcolorModAvalible::_16Colors: {
        std::string colorFront{std::format(ActionTable::string_16ColorFGMod, 
            static_cast<int>(transfor16ColorMod(RGBFront_)))};
        std::string colorBack{std::format(ActionTable::string_16ColorBGMod, 
            static_cast<int>(transfor16ColorMod(RGBBack_)))};
        result = colorFront + colorBack;
        break;
    }

    case TUIcolorModAvalible::_256Colors: { // not supported yet 16 color mod logic
        std::string colorFront{std::format(ActionTable::string_16ColorFGMod, 
            static_cast<int>(transfor16ColorMod(RGBFront_)))};
        std::string colorBack{std::format(ActionTable::string_16ColorBGMod, 
            static_cast<int>(transfor16ColorMod(RGBBack_)))};
        result = colorFront + colorBack;
        break;
    }

    case TUIcolorModAvalible::_trueColor: {
        std::string colorFront{std::format(ActionTable::string_trueColorFGMod, 
            static_cast<int>(RGBFront_.R_), 
            static_cast<int>(RGBFront_.G_), 
            static_cast<int>(RGBFront_.B_))};
        std::string colorBack{std::format(ActionTable::string_trueColorBGMod, 
            static_cast<int>(RGBBack_.R_), 
            static_cast<int>(RGBBack_.G_), 
            static_cast<int>(RGBBack_.B_))};
        result = colorFront + colorBack;
        break;
    }
    
    default:
        break;
    }
    
    return result;
}

} // namespace
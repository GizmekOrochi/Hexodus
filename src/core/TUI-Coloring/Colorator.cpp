#include "../../include/TUI-Coloring/Colorator.hpp"

namespace TUI {

Colorator::Colorator() : RGBFront_(), RGBBack_(), ColorMod_() {
    if (const char* colorterm = getenv("COLORTERM")) {
        if (strstr(colorterm, "truecolor") || strstr(colorterm, "24bit")) {
            ColorMod_ = TUIcolorModAvalible::_trueColor;
            return;
        }
    }

    else if (const char* term = getenv("TERM")) {
        if (strstr(term, "256color")) {
            ColorMod_ = TUIcolorModAvalible::_256Colors;
            return;
        }
    }

    else ColorMod_ = TUIcolorModAvalible::_16Colors;
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


} // namespace
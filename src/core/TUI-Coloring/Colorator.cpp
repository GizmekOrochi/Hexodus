#include "../../include/TUI-Coloring/Colorator.hpp"
#include <unistd.h>
#include <cerrno>
#include <stdexcept>

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

namespace {
std::string ansi16(RGBPanel color, bool background) {
    const int index = static_cast<int>(transfor16ColorMod(color));
    const int code = (index < 8 ? (background ? 40 : 30) : (background ? 100 : 90)) + index % 8;
    return "\x1b[" + std::to_string(code) + "m";
}
}

void Colorator::ApplyColor() {
    const std::string output = GetColorString();
    std::size_t offset = 0;
    while (offset < output.size()) {
        const auto n = ::write(STDOUT_FILENO, output.data() + offset, output.size() - offset);
        if (n > 0) offset += static_cast<std::size_t>(n);
        else if (n < 0 && errno == EINTR) continue;
        else throw std::runtime_error("Colorator: write failed");
    }
}

std::string Colorator::GetColorString() const {
    if (ColorMod_ == TUIcolorModAvalible::_trueColor) {
        return std::format("\x1b[38;2;{};{};{}m\x1b[48;2;{};{};{}m",
            RGBFront_.R_, RGBFront_.G_, RGBFront_.B_, RGBBack_.R_, RGBBack_.G_, RGBBack_.B_);
    }
    // The 256-color mode still intentionally uses the documented 16-color fallback.
    return ansi16(RGBFront_, false) + ansi16(RGBBack_, true);
}

} // namespace TUI

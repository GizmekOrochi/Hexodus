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
    const std::string term = getenv("TERM") ? getenv("TERM") : "";
    const std::string ct = getenv("COLORTERM") ? getenv("COLORTERM") : "";
    const std::string override = getenv("HEXODUS_COLORS") ? getenv("HEXODUS_COLORS") : "auto";
    if (override != "auto" && override != "16" && override != "256" && override != "truecolor")
        throw std::invalid_argument("HEXODUS_COLORS: expected auto, 16, 256 or truecolor");
    // A console must not inherit truecolor support from a graphical session.
    if (term == "linux" || term.starts_with("linux-")) ColorMod_ = _16Colors;
    else if (override == "truecolor") ColorMod_ = _trueColor;
    else if (override == "256") ColorMod_ = _256Colors;
    else if (override == "16") ColorMod_ = _16Colors;
    else if (ct == "truecolor" || ct == "24bit") ColorMod_ = _trueColor;
    else if (term.find("256color") != std::string::npos) ColorMod_ = _256Colors;
    else ColorMod_ = _16Colors;
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
    if (ColorMod_ == _256Colors) {
        const auto index = [](RGBPanel c) {
            int best = 16, bestError = 1000000;
            // Avoid customizable ANSI indices 0..15: use the standard cube and grays.
            for (int i = 16; i < 256; ++i) {
                const int n = i - 16;
                const auto level = [](int v) { return v == 0 ? 0 : 55 + 40*v; };
                const int red = i < 232 ? level(n/36) : 8+10*(i-232);
                const int green = i < 232 ? level((n/6)%6) : red;
                const int blue = i < 232 ? level(n%6) : red;
                const int dr=int(c.R_)-red,dg=int(c.G_)-green,db=int(c.B_)-blue;
                const int error=dr*dr+dg*dg+db*db;
                if(error<bestError) { bestError=error; best=i; }
            }
            return best;
        };
        return std::format("\x1b[38;5;{}m\x1b[48;5;{}m",index(RGBFront_),index(RGBBack_));
    }
    return ansi16(RGBFront_, false) + ansi16(RGBBack_, true);
}

} // namespace TUI

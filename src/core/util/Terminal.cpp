#include "../../include/util/Terminal.hpp"

#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace TUI {

TerminalSize Terminal::querySize() {
    winsize ws{};

    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        throw std::runtime_error(
            "TUI::Terminal: unable to query terminal size"
        );
    }

    return TerminalSize{
        static_cast<int>(ws.ws_col),
        static_cast<int>(ws.ws_row)
    };
}

Terminal::Terminal()
    : size_{querySize()},
      previousSize_{size_} {}

void Terminal::update() {
    previousSize_ = size_;
    size_ = querySize();
    resized_ = size_ != previousSize_;
}

const TerminalSize& Terminal::size() const noexcept {
    return size_;
}

const TerminalSize& Terminal::previousSize() const noexcept {
    return previousSize_;
}

int Terminal::width() const noexcept {
    return size_.width;
}

int Terminal::height() const noexcept {
    return size_.height;
}

bool Terminal::resized() const noexcept {
    return resized_;
}

void Terminal::setMinimumSize(TerminalSize size) {
    setMinimumSize(size.width, size.height);
}

void Terminal::setMinimumSize(int width, int height) {
    minimumSize_.width = std::max(width, 1);
    minimumSize_.height = std::max(height, 1);
}

const TerminalSize& Terminal::minimumSize() const noexcept {
    return minimumSize_;
}

bool Terminal::minimumSizeSatisfied() const noexcept {
    return size_.width >= minimumSize_.width &&
           size_.height >= minimumSize_.height;
}

} // namespace TUI

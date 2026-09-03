#ifndef TUI_TERMINAL_HPP
#define TUI_TERMINAL_HPP

#pragma once

#include <algorithm>

namespace TUI {

struct TerminalSize {
    int width{0};
    int height{0};

    [[nodiscard]] bool valid() const noexcept {
        return width > 0 && height > 0;
    }

    [[nodiscard]] bool operator==(const TerminalSize&) const noexcept = default;
};

class Terminal {
private:
    TerminalSize size_{};
    TerminalSize previousSize_{};
    TerminalSize minimumSize_{1, 1};
    bool resized_{false};

    [[nodiscard]] static TerminalSize querySize();

public:
    Terminal();

    // Relit la taille physique du terminal.
    // resized() reste vrai jusqu'au prochain update().
    void update();

    [[nodiscard]] const TerminalSize& size() const noexcept;
    [[nodiscard]] const TerminalSize& previousSize() const noexcept;

    [[nodiscard]] int width() const noexcept;
    [[nodiscard]] int height() const noexcept;

    [[nodiscard]] bool resized() const noexcept;

    void setMinimumSize(TerminalSize size);
    void setMinimumSize(int width, int height);

    [[nodiscard]] const TerminalSize& minimumSize() const noexcept;
    [[nodiscard]] bool minimumSizeSatisfied() const noexcept;
};

} // namespace TUI

#endif // TUI_TERMINAL_HPP

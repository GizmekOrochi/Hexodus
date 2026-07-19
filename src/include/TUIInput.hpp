#ifndef TUIINPUT_HPP
#define TUIINPUT_HPP

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <optional>
#include <string>
#include <termios.h>

namespace TUI {

enum class Key : std::uint16_t {
    Unknown = 0,
    Escape,
    Enter,
    Tab,
    Backspace,
    Space,

    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,

    Home,
    End,
    Insert,
    Delete,
    PageUp,
    PageDown,

    F1, F2, F3, F4, F5, F6,
    F7, F8, F9, F10, F11, F12,

    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,

    Count
};

class TUIInput {
public:
    TUIInput() = default;
    ~TUIInput();

    TUIInput(const TUIInput&) = delete;
    TUIInput& operator=(const TUIInput&) = delete;

    // Enables raw, non-blocking keyboard input.
    // This function does not enable any mouse terminal protocol.
    void activate();

    // Restores the exact stdin flags and termios state saved by activate().
    void restore() noexcept;

    // Reads every keyboard event currently waiting on stdin.
    // This call never blocks.
    void pollEvents();

    [[nodiscard]] bool active() const noexcept {
        return active_;
    }

    // True when this key was received during the current poll.
    [[nodiscard]] bool keyPressed(Key key) const noexcept;

    // In a classic terminal there is no key-release event.
    // keyDown therefore means "received this frame or from OS key repeat".
    [[nodiscard]] bool keyDown(Key key) const noexcept;

    [[nodiscard]] bool quitRequested() const noexcept {
        return quitRequested_;
    }

    void requestQuit() noexcept {
        quitRequested_ = true;
    }

    void clearQuitRequest() noexcept {
        quitRequested_ = false;
    }

private:
    static constexpr std::size_t keyCount_{static_cast<std::size_t>(Key::Count)};

    bool active_{false};
    bool quitRequested_{false};
    termios originalTermios_{};

    std::array<bool, keyCount_> pressed_{};
    std::array<bool, keyCount_> down_{};

    // Keeps an incomplete ANSI escape sequence between two polls.
    using Clock = std::chrono::steady_clock;
    static constexpr auto escapeDelay_{std::chrono::milliseconds{30}};

    std::string pending_{};
    std::optional<Clock::time_point> pendingEscapeSince_{};

    void resetFrameState() noexcept;
    void readAvailableInput();
    void parsePendingInput();

    std::size_t parseEscapeSequence(std::size_t offset);
    std::size_t parseCsiSequence(std::size_t offset);

    void registerByte(unsigned char byte);
    void registerKey(Key key) noexcept;

    [[nodiscard]] static Key keyFromAscii(unsigned char byte) noexcept;
    [[nodiscard]] static Key keyFromCsiNumber(int number) noexcept;
};

} // namespace TUI

#endif // TUIINPUT_HPP

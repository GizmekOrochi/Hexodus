#ifndef TUI_INPUT_HPP
#define TUI_INPUT_HPP


#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <termios.h>
#include <vector>

namespace TUI {

enum class Scancode : std::uint16_t {
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

    LeftShift,
    RightShift,
    LeftControl,
    RightControl,
    LeftAlt,
    RightAlt,
    LeftSuper,
    RightSuper,
    Menu,

    CapsLock,
    NumLock,
    ScrollLock,
    PrintScreen,
    Pause,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,

    KeyA,
    KeyB,
    KeyC,
    KeyD,
    KeyE,
    KeyF,
    KeyG,
    KeyH,
    KeyI,
    KeyJ,
    KeyK,
    KeyL,
    KeyM,
    KeyN,
    KeyO,
    KeyP,
    KeyQ,
    KeyR,
    KeyS,
    KeyT,
    KeyU,
    KeyV,
    KeyW,
    KeyX,
    KeyY,
    KeyZ,

    Digit0,
    Digit1,
    Digit2,
    Digit3,
    Digit4,
    Digit5,
    Digit6,
    Digit7,
    Digit8,
    Digit9,

    Grave,
    Minus,
    Equal,
    LeftBracket,
    RightBracket,
    Backslash,
    Semicolon,
    Apostrophe,
    Comma,
    Period,
    Slash,

    Keypad0,
    Keypad1,
    Keypad2,
    Keypad3,
    Keypad4,
    Keypad5,
    Keypad6,
    Keypad7,
    Keypad8,
    Keypad9,

    KeypadDecimal,
    KeypadDivide,
    KeypadMultiply,
    KeypadSubtract,
    KeypadAdd,
    KeypadEnter,
    KeypadEqual,

    VolumeMute,
    VolumeDown,
    VolumeUp,
    MediaPlayPause,
    MediaStop,
    MediaPrevious,
    MediaNext,

    Count
};

class TUIInput {
public:
    TUIInput();
    ~TUIInput();

    TUIInput(const TUIInput&) = delete;
    TUIInput& operator=(const TUIInput&) = delete;

    void activate();
    void restore() noexcept;
    void pollEvents();

    [[nodiscard]] bool isActive() const noexcept;
    [[nodiscard]] bool quitRequested() const noexcept;

    void requestQuit() noexcept;
    void clearQuitRequest() noexcept;

    [[nodiscard]] bool pressed(Scancode scancode) const noexcept;
    [[nodiscard]] bool repeated(Scancode scancode) const noexcept;
    [[nodiscard]] bool released(Scancode scancode) const noexcept;
    [[nodiscard]] bool down(Scancode scancode) const noexcept;

    [[nodiscard]] std::size_t keyboardCount() const noexcept;

private:
    static constexpr std::size_t scancodeCount_{
        static_cast<std::size_t>(Scancode::Count)
    };

    struct KeyboardDevice {
        int fileDescriptor;
        std::string path;
        std::string name;
        bool synchronized;
    };

    bool active_;
    bool quitRequested_;
    bool terminalConfigured_;

    termios originalTerminalSettings_;

    std::vector<KeyboardDevice> keyboards_;

    std::array<bool, scancodeCount_> pressed_;
    std::array<bool, scancodeCount_> repeated_;
    std::array<bool, scancodeCount_> released_;
    std::array<bool, scancodeCount_> down_;

    void configureTerminal();
    void restoreTerminal() noexcept;
    void drainTerminalInput() noexcept;

    void discoverKeyboards();
    void closeKeyboards() noexcept;

    void clearFrameStates() noexcept;
    void readKeyboardEvents(KeyboardDevice& keyboard);
    void resynchronizeKeyboard(KeyboardDevice& keyboard);

    void registerEvent(std::uint16_t linuxCode, std::int32_t value) noexcept;

    [[nodiscard]] static bool isKeyboard(int fileDescriptor) noexcept;
    [[nodiscard]] static std::string keyboardName(int fileDescriptor);
    [[nodiscard]] static Scancode fromLinuxCode(std::uint16_t linuxCode) noexcept;
};

} // namespace TUI

#endif // TUI_INPUT_HPP

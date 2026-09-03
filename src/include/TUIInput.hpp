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

enum class MouseButton : std::uint8_t {
    Left = 0,
    Right,
    Middle,
    Back,
    Forward,

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
    [[nodiscard]] std::size_t mouseCount() const noexcept;

    void setMouseBounds(int width, int height) noexcept;
    void setMousePosition(int x, int y) noexcept;
    void setMouseSensitivity(float sensitivity) noexcept;

    [[nodiscard]] float mouseSensitivity() const noexcept;

    [[nodiscard]] int mouseX() const noexcept;
    [[nodiscard]] int mouseY() const noexcept;
    [[nodiscard]] int mouseDeltaX() const noexcept;
    [[nodiscard]] int mouseDeltaY() const noexcept;
    [[nodiscard]] int mouseWheelDelta() const noexcept;

    [[nodiscard]] bool mousePressed(MouseButton button) const noexcept;
    [[nodiscard]] bool mouseReleased(MouseButton button) const noexcept;
    [[nodiscard]] bool mouseDown(MouseButton button) const noexcept;

    [[nodiscard]] bool mouseLeftDown() const noexcept;
    [[nodiscard]] bool mouseRightDown() const noexcept;
    [[nodiscard]] bool mouseMiddleDown() const noexcept;

    [[nodiscard]] bool mouseLeftPressed() const noexcept;
    [[nodiscard]] bool mouseRightPressed() const noexcept;
    [[nodiscard]] bool mouseMiddlePressed() const noexcept;

    [[nodiscard]] bool mouseLeftReleased() const noexcept;
    [[nodiscard]] bool mouseRightReleased() const noexcept;
    [[nodiscard]] bool mouseMiddleReleased() const noexcept;

    [[nodiscard]] bool mouseBackDown() const noexcept;
    [[nodiscard]] bool mouseForwardDown() const noexcept;
    [[nodiscard]] bool mouseBackPressed() const noexcept;
    [[nodiscard]] bool mouseForwardPressed() const noexcept;
    [[nodiscard]] bool mouseBackReleased() const noexcept;
    [[nodiscard]] bool mouseForwardReleased() const noexcept;

private:
    static constexpr std::size_t scancodeCount_{static_cast<std::size_t>(Scancode::Count)};

    struct KeyboardDevice {
        int fileDescriptor;
        std::string path;
        std::string name;
        bool synchronized;
    };

    struct MouseDevice {
        int fileDescriptor;
        std::string path;
        std::string name;
        bool synchronized;
    };

    struct MouseState {
        int x;
        int y;
        int deltaX;
        int deltaY;
        int wheelDelta;

        int width;
        int height;

        float sensitivity;
        float residualX;
        float residualY;

        std::array<bool, static_cast<std::size_t>(MouseButton::Count)> down;
        std::array<bool, static_cast<std::size_t>(MouseButton::Count)> pressed;
        std::array<bool, static_cast<std::size_t>(MouseButton::Count)> released;
    };

    bool active_;
    bool quitRequested_;
    bool terminalConfigured_;

    termios originalTerminalSettings_;

    std::vector<KeyboardDevice> keyboards_;
    std::vector<MouseDevice> mice_;

    MouseState mouse_;

    std::array<bool, scancodeCount_> pressed_;
    std::array<bool, scancodeCount_> repeated_;
    std::array<bool, scancodeCount_> released_;
    std::array<bool, scancodeCount_> down_;

    void configureTerminal();
    void restoreTerminal() noexcept;
    void drainTerminalInput() noexcept;

    void discoverDevices();
    void closeDevices() noexcept;

    void clearFrameStates() noexcept;
    void readKeyboardEvents(KeyboardDevice& keyboard);
    void readMouseEvents(MouseDevice& mouse);
    void resynchronizeKeyboard(KeyboardDevice& keyboard);
    void resynchronizeMouse(MouseDevice& mouse);

    void registerEvent(std::uint16_t linuxCode, std::int32_t value) noexcept;
    void registerMouseButton(std::uint16_t linuxCode, std::int32_t value) noexcept;
    void clampMousePosition() noexcept;

    [[nodiscard]] static bool isKeyboard(int fileDescriptor) noexcept;
    [[nodiscard]] static bool isMouse(int fileDescriptor) noexcept;
    [[nodiscard]] static std::string deviceName(int fileDescriptor);
    [[nodiscard]] static Scancode fromLinuxCode(std::uint16_t linuxCode) noexcept;
    [[nodiscard]] static MouseButton fromLinuxMouseButton(std::uint16_t linuxCode) noexcept;
};

} // namespace TUI

#endif // TUI_INPUT_HPP

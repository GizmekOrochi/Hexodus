#include "../include/TUIInput.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <linux/input.h>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

namespace TUI {

namespace {

constexpr std::size_t bitsPerWord{sizeof(unsigned long) * 8U};

constexpr std::size_t wordCountForBit(std::size_t maximumBit) noexcept {
    return (maximumBit + bitsPerWord) / bitsPerWord;
}

bool bitIsSet(const unsigned long* bits, std::size_t bit) noexcept {
    return (bits[bit / bitsPerWord] & (1UL << (bit % bitsPerWord))) != 0UL;
}

std::string makeSystemError(const std::string& message) {
    return message + ": " + std::strerror(errno);
}

} // namespace

TUIInput::TUIInput()
    : active_{false},
      quitRequested_{false},
      terminalConfigured_{false},
      originalTerminalSettings_{},
      keyboards_{},
      pressed_{},
      repeated_{},
      released_{},
      down_{} {}

TUIInput::~TUIInput() {
    restore();
}

void TUIInput::activate() {
    if (active_) {
        return;
    }

    pressed_.fill(false);
    repeated_.fill(false);
    released_.fill(false);
    down_.fill(false);

    quitRequested_ = false;

    try {
        configureTerminal();
        discoverKeyboards();
        active_ = true;
    } catch (...) {
        closeKeyboards();
        restoreTerminal();
        throw;
    }
}

void TUIInput::restore() noexcept {
    closeKeyboards();
    restoreTerminal();

    pressed_.fill(false);
    repeated_.fill(false);
    released_.fill(false);
    down_.fill(false);

    active_ = false;
}

void TUIInput::pollEvents() {
    if (!active_) {
        throw std::logic_error("TUIInput::pollEvents(): input is not active");
    }

    clearFrameStates();
    drainTerminalInput();

    std::vector<pollfd> descriptors{};
    descriptors.reserve(keyboards_.size());

    for (const KeyboardDevice& keyboard : keyboards_) {
        descriptors.push_back(pollfd{keyboard.fileDescriptor, POLLIN, 0});
    }

    while (true) {
        const int readyCount{
            ::poll(descriptors.data(), static_cast<nfds_t>(descriptors.size()), 0)
        };

        if (readyCount < 0) {
            if (errno == EINTR) {
                continue;
            }

            throw std::runtime_error(
                makeSystemError("TUIInput::pollEvents(): poll failed")
            );
        }

        if (readyCount == 0) {
            break;
        }

        bool eventRead{false};

        for (std::size_t index{}; index < descriptors.size(); ++index) {
            if ((descriptors[index].revents & POLLIN) == 0) {
                continue;
            }

            readKeyboardEvents(keyboards_[index]);
            eventRead = true;
        }

        if (!eventRead) {
            break;
        }

        for (pollfd& descriptor : descriptors) {
            descriptor.revents = 0;
        }
    }

    const bool controlDown{
        down(Scancode::LeftControl) || down(Scancode::RightControl)
    };

    if (controlDown && pressed(Scancode::KeyC)) {
        quitRequested_ = true;
    }
}

bool TUIInput::isActive() const noexcept {
    return active_;
}

bool TUIInput::quitRequested() const noexcept {
    return quitRequested_;
}

void TUIInput::requestQuit() noexcept {
    quitRequested_ = true;
}

void TUIInput::clearQuitRequest() noexcept {
    quitRequested_ = false;
}

bool TUIInput::pressed(Scancode scancode) const noexcept {
    const std::size_t index{static_cast<std::size_t>(scancode)};
    return index < scancodeCount_ && pressed_[index];
}

bool TUIInput::repeated(Scancode scancode) const noexcept {
    const std::size_t index{static_cast<std::size_t>(scancode)};
    return index < scancodeCount_ && repeated_[index];
}

bool TUIInput::released(Scancode scancode) const noexcept {
    const std::size_t index{static_cast<std::size_t>(scancode)};
    return index < scancodeCount_ && released_[index];
}

bool TUIInput::down(Scancode scancode) const noexcept {
    const std::size_t index{static_cast<std::size_t>(scancode)};
    return index < scancodeCount_ && down_[index];
}

std::size_t TUIInput::keyboardCount() const noexcept {
    return keyboards_.size();
}

void TUIInput::configureTerminal() {
    if (!::isatty(STDIN_FILENO)) {
        return;
    }

    if (::tcgetattr(STDIN_FILENO, &originalTerminalSettings_) == -1) {
        throw std::runtime_error(
            makeSystemError("TUIInput::configureTerminal(): tcgetattr failed")
        );
    }

    termios raw{originalTerminalSettings_};

    raw.c_iflag &= static_cast<tcflag_t>(
        ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON)
    );

    raw.c_cflag |= CS8;

    raw.c_lflag &= static_cast<tcflag_t>(
        ~(ECHO | ICANON | IEXTEN | ISIG)
    );

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        throw std::runtime_error(
            makeSystemError("TUIInput::configureTerminal(): tcsetattr failed")
        );
    }

    terminalConfigured_ = true;
}

void TUIInput::restoreTerminal() noexcept {
    if (!terminalConfigured_) {
        return;
    }

    ::tcsetattr(
        STDIN_FILENO,
        TCSAFLUSH,
        &originalTerminalSettings_
    );

    terminalConfigured_ = false;
}

void TUIInput::drainTerminalInput() noexcept {
    if (!terminalConfigured_) {
        return;
    }

    std::array<char, 256> buffer{};

    while (true) {
        const ssize_t byteCount{
            ::read(STDIN_FILENO, buffer.data(), buffer.size())
        };

        if (byteCount > 0) {
            continue;
        }

        if (byteCount < 0 && errno == EINTR) {
            continue;
        }

        break;
    }
}

void TUIInput::discoverKeyboards() {
    namespace fs = std::filesystem;

    const fs::path inputDirectory{"/dev/input"};
    std::error_code error{};

    if (!fs::exists(inputDirectory, error)) {
        throw std::runtime_error(
            "TUIInput::discoverKeyboards(): /dev/input does not exist"
        );
    }

    std::vector<fs::path> candidates{};

    for (const fs::directory_entry& entry :
         fs::directory_iterator(inputDirectory, error)) {
        if (error) {
            break;
        }

        if (!entry.is_character_file(error)) {
            continue;
        }

        const std::string filename{entry.path().filename().string()};

        if (filename.rfind("event", 0U) == 0U) {
            candidates.push_back(entry.path());
        }
    }

    std::sort(candidates.begin(), candidates.end());

    bool permissionDenied{false};

    for (const fs::path& path : candidates) {
        const int fileDescriptor{
            ::open(path.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC)
        };

        if (fileDescriptor == -1) {
            if (errno == EACCES || errno == EPERM) {
                permissionDenied = true;
            }

            continue;
        }

        if (!isKeyboard(fileDescriptor)) {
            ::close(fileDescriptor);
            continue;
        }

        keyboards_.push_back(
            KeyboardDevice{
                fileDescriptor,
                path.string(),
                keyboardName(fileDescriptor),
                true
            }
        );
    }

    if (keyboards_.empty()) {
        if (permissionDenied) {
            throw std::runtime_error(
                "TUIInput::discoverKeyboards(): no readable keyboard found; "
                "grant read access to /dev/input/event*"
            );
        }

        throw std::runtime_error(
            "TUIInput::discoverKeyboards(): no evdev keyboard found"
        );
    }

    for (KeyboardDevice& keyboard : keyboards_) {
        resynchronizeKeyboard(keyboard);
    }
}

void TUIInput::closeKeyboards() noexcept {
    for (KeyboardDevice& keyboard : keyboards_) {
        if (keyboard.fileDescriptor == -1) {
            continue;
        }

        ::close(keyboard.fileDescriptor);
        keyboard.fileDescriptor = -1;
    }

    keyboards_.clear();
}

void TUIInput::clearFrameStates() noexcept {
    pressed_.fill(false);
    repeated_.fill(false);
    released_.fill(false);
}

void TUIInput::readKeyboardEvents(KeyboardDevice& keyboard) {
    std::array<input_event, 64> events{};

    while (true) {
        const ssize_t byteCount{
            ::read(
                keyboard.fileDescriptor,
                events.data(),
                sizeof(events)
            )
        };

        if (byteCount < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }

            throw std::runtime_error(
                makeSystemError(
                    "TUIInput::readKeyboardEvents(): read failed for " +
                    keyboard.path
                )
            );
        }

        if (byteCount == 0) {
            return;
        }

        const std::size_t eventCount{
            static_cast<std::size_t>(byteCount) / sizeof(input_event)
        };

        for (std::size_t index{}; index < eventCount; ++index) {
            const input_event& event{events[index]};

            if (event.type == EV_SYN) {
                if (event.code == SYN_DROPPED) {
                    keyboard.synchronized = false;
                } else if (
                    event.code == SYN_REPORT &&
                    !keyboard.synchronized
                ) {
                    resynchronizeKeyboard(keyboard);
                    keyboard.synchronized = true;
                }

                continue;
            }

            if (!keyboard.synchronized) {
                continue;
            }

            if (event.type == EV_KEY) {
                registerEvent(event.code, event.value);
            }
        }

        if (static_cast<std::size_t>(byteCount) < sizeof(events)) {
            return;
        }
    }
}

void TUIInput::resynchronizeKeyboard(KeyboardDevice& keyboard) {
    std::array<unsigned long, wordCountForBit(KEY_MAX)> keyState{};

    if (::ioctl(
        keyboard.fileDescriptor,
        EVIOCGKEY(sizeof(keyState)),
        keyState.data()
    ) == -1) {
        return;
    }

    for (std::uint16_t code{}; code <= KEY_MAX; ++code) {
        const Scancode scancode{fromLinuxCode(code)};

        if (scancode == Scancode::Unknown) {
            continue;
        }

        const std::size_t index{static_cast<std::size_t>(scancode)};
        const bool isDown{bitIsSet(keyState.data(), code)};

        if (isDown && !down_[index]) {
            down_[index] = true;
            pressed_[index] = true;
        } else if (!isDown && down_[index]) {
            down_[index] = false;
            released_[index] = true;
        }
    }
}

void TUIInput::registerEvent(
    std::uint16_t linuxCode,
    std::int32_t value
) noexcept {
    const Scancode scancode{fromLinuxCode(linuxCode)};

    if (scancode == Scancode::Unknown) {
        return;
    }

    const std::size_t index{static_cast<std::size_t>(scancode)};

    switch (value) {
        case 0:
            if (down_[index]) {
                released_[index] = true;
            }

            down_[index] = false;
            break;

        case 1:
            if (!down_[index]) {
                pressed_[index] = true;
            }

            down_[index] = true;
            break;

        case 2:
            repeated_[index] = true;
            down_[index] = true;
            break;

        default:
            break;
    }
}

bool TUIInput::isKeyboard(int fileDescriptor) noexcept {
    std::array<unsigned long, wordCountForBit(EV_MAX)> eventBits{};

    if (::ioctl(
        fileDescriptor,
        EVIOCGBIT(0, sizeof(eventBits)),
        eventBits.data()
    ) == -1) {
        return false;
    }

    if (!bitIsSet(eventBits.data(), EV_KEY)) {
        return false;
    }

    std::array<unsigned long, wordCountForBit(KEY_MAX)> keyBits{};

    if (::ioctl(
        fileDescriptor,
        EVIOCGBIT(EV_KEY, sizeof(keyBits)),
        keyBits.data()
    ) == -1) {
        return false;
    }

    return bitIsSet(keyBits.data(), KEY_A) &&
           bitIsSet(keyBits.data(), KEY_Z) &&
           bitIsSet(keyBits.data(), KEY_ENTER) &&
           bitIsSet(keyBits.data(), KEY_SPACE);
}

std::string TUIInput::keyboardName(int fileDescriptor) {
    std::array<char, 256> name{};

    if (::ioctl(
        fileDescriptor,
        EVIOCGNAME(name.size()),
        name.data()
    ) == -1) {
        return "Unknown keyboard";
    }

    return name.data();
}

Scancode TUIInput::fromLinuxCode(std::uint16_t linuxCode) noexcept {
    switch (linuxCode) {
        case KEY_ESC: return Scancode::Escape;
        case KEY_ENTER: return Scancode::Enter;
        case KEY_TAB: return Scancode::Tab;
        case KEY_BACKSPACE: return Scancode::Backspace;
        case KEY_SPACE: return Scancode::Space;

        case KEY_UP: return Scancode::ArrowUp;
        case KEY_DOWN: return Scancode::ArrowDown;
        case KEY_LEFT: return Scancode::ArrowLeft;
        case KEY_RIGHT: return Scancode::ArrowRight;

        case KEY_HOME: return Scancode::Home;
        case KEY_END: return Scancode::End;
        case KEY_INSERT: return Scancode::Insert;
        case KEY_DELETE: return Scancode::Delete;
        case KEY_PAGEUP: return Scancode::PageUp;
        case KEY_PAGEDOWN: return Scancode::PageDown;

        case KEY_LEFTSHIFT: return Scancode::LeftShift;
        case KEY_RIGHTSHIFT: return Scancode::RightShift;
        case KEY_LEFTCTRL: return Scancode::LeftControl;
        case KEY_RIGHTCTRL: return Scancode::RightControl;
        case KEY_LEFTALT: return Scancode::LeftAlt;
        case KEY_RIGHTALT: return Scancode::RightAlt;
        case KEY_LEFTMETA: return Scancode::LeftSuper;
        case KEY_RIGHTMETA: return Scancode::RightSuper;
        case KEY_MENU: return Scancode::Menu;

        case KEY_CAPSLOCK: return Scancode::CapsLock;
        case KEY_NUMLOCK: return Scancode::NumLock;
        case KEY_SCROLLLOCK: return Scancode::ScrollLock;
        case KEY_SYSRQ: return Scancode::PrintScreen;
        case KEY_PAUSE: return Scancode::Pause;

        case KEY_F1: return Scancode::F1;
        case KEY_F2: return Scancode::F2;
        case KEY_F3: return Scancode::F3;
        case KEY_F4: return Scancode::F4;
        case KEY_F5: return Scancode::F5;
        case KEY_F6: return Scancode::F6;
        case KEY_F7: return Scancode::F7;
        case KEY_F8: return Scancode::F8;
        case KEY_F9: return Scancode::F9;
        case KEY_F10: return Scancode::F10;
        case KEY_F11: return Scancode::F11;
        case KEY_F12: return Scancode::F12;
        case KEY_F13: return Scancode::F13;
        case KEY_F14: return Scancode::F14;
        case KEY_F15: return Scancode::F15;
        case KEY_F16: return Scancode::F16;
        case KEY_F17: return Scancode::F17;
        case KEY_F18: return Scancode::F18;
        case KEY_F19: return Scancode::F19;
        case KEY_F20: return Scancode::F20;
        case KEY_F21: return Scancode::F21;
        case KEY_F22: return Scancode::F22;
        case KEY_F23: return Scancode::F23;
        case KEY_F24: return Scancode::F24;

        case KEY_A: return Scancode::KeyA;
        case KEY_B: return Scancode::KeyB;
        case KEY_C: return Scancode::KeyC;
        case KEY_D: return Scancode::KeyD;
        case KEY_E: return Scancode::KeyE;
        case KEY_F: return Scancode::KeyF;
        case KEY_G: return Scancode::KeyG;
        case KEY_H: return Scancode::KeyH;
        case KEY_I: return Scancode::KeyI;
        case KEY_J: return Scancode::KeyJ;
        case KEY_K: return Scancode::KeyK;
        case KEY_L: return Scancode::KeyL;
        case KEY_M: return Scancode::KeyM;
        case KEY_N: return Scancode::KeyN;
        case KEY_O: return Scancode::KeyO;
        case KEY_P: return Scancode::KeyP;
        case KEY_Q: return Scancode::KeyQ;
        case KEY_R: return Scancode::KeyR;
        case KEY_S: return Scancode::KeyS;
        case KEY_T: return Scancode::KeyT;
        case KEY_U: return Scancode::KeyU;
        case KEY_V: return Scancode::KeyV;
        case KEY_W: return Scancode::KeyW;
        case KEY_X: return Scancode::KeyX;
        case KEY_Y: return Scancode::KeyY;
        case KEY_Z: return Scancode::KeyZ;

        case KEY_0: return Scancode::Digit0;
        case KEY_1: return Scancode::Digit1;
        case KEY_2: return Scancode::Digit2;
        case KEY_3: return Scancode::Digit3;
        case KEY_4: return Scancode::Digit4;
        case KEY_5: return Scancode::Digit5;
        case KEY_6: return Scancode::Digit6;
        case KEY_7: return Scancode::Digit7;
        case KEY_8: return Scancode::Digit8;
        case KEY_9: return Scancode::Digit9;

        case KEY_GRAVE: return Scancode::Grave;
        case KEY_MINUS: return Scancode::Minus;
        case KEY_EQUAL: return Scancode::Equal;
        case KEY_LEFTBRACE: return Scancode::LeftBracket;
        case KEY_RIGHTBRACE: return Scancode::RightBracket;
        case KEY_BACKSLASH: return Scancode::Backslash;
        case KEY_SEMICOLON: return Scancode::Semicolon;
        case KEY_APOSTROPHE: return Scancode::Apostrophe;
        case KEY_COMMA: return Scancode::Comma;
        case KEY_DOT: return Scancode::Period;
        case KEY_SLASH: return Scancode::Slash;

        case KEY_KP0: return Scancode::Keypad0;
        case KEY_KP1: return Scancode::Keypad1;
        case KEY_KP2: return Scancode::Keypad2;
        case KEY_KP3: return Scancode::Keypad3;
        case KEY_KP4: return Scancode::Keypad4;
        case KEY_KP5: return Scancode::Keypad5;
        case KEY_KP6: return Scancode::Keypad6;
        case KEY_KP7: return Scancode::Keypad7;
        case KEY_KP8: return Scancode::Keypad8;
        case KEY_KP9: return Scancode::Keypad9;

        case KEY_KPDOT: return Scancode::KeypadDecimal;
        case KEY_KPSLASH: return Scancode::KeypadDivide;
        case KEY_KPASTERISK: return Scancode::KeypadMultiply;
        case KEY_KPMINUS: return Scancode::KeypadSubtract;
        case KEY_KPPLUS: return Scancode::KeypadAdd;
        case KEY_KPENTER: return Scancode::KeypadEnter;
        case KEY_KPEQUAL: return Scancode::KeypadEqual;

        case KEY_MUTE: return Scancode::VolumeMute;
        case KEY_VOLUMEDOWN: return Scancode::VolumeDown;
        case KEY_VOLUMEUP: return Scancode::VolumeUp;
        case KEY_PLAYPAUSE: return Scancode::MediaPlayPause;
        case KEY_STOPCD: return Scancode::MediaStop;
        case KEY_PREVIOUSSONG: return Scancode::MediaPrevious;
        case KEY_NEXTSONG: return Scancode::MediaNext;

        default:
            return Scancode::Unknown;
    }
}

} // namespace TUI

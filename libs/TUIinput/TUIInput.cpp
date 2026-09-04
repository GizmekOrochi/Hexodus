#include "TUIInput.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <cmath>
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
      keyboardLayout_{KeyboardLayout::Physical},
      originalTerminalSettings_{},
      keyboards_{},
      mice_{},
      mouse_{
          0,
          0,
          0,
          0,
          0,
          1,
          1,
          1.0f,
          0.0f,
          0.0f,
          {},
          {},
          {}
      },
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

    mouse_.deltaX = 0;
    mouse_.deltaY = 0;
    mouse_.wheelDelta = 0;

    mouse_.down.fill(false);
    mouse_.pressed.fill(false);
    mouse_.released.fill(false);

    quitRequested_ = false;

    try {
        configureTerminal();
        discoverDevices();
        active_ = true;
    } catch (...) {
        closeDevices();
        restoreTerminal();
        throw;
    }
}

void TUIInput::restore() noexcept {
    closeDevices();
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

    struct PolledDevice {
        int fileDescriptor;
        bool mouse;
        std::size_t index;
    };

    std::vector<PolledDevice> devices{};
    devices.reserve(keyboards_.size() + mice_.size());

    for (std::size_t index{}; index < keyboards_.size(); ++index) {
        devices.push_back(
            PolledDevice{
                keyboards_[index].fileDescriptor,
                false,
                index
            }
        );
    }

    for (std::size_t index{}; index < mice_.size(); ++index) {
        devices.push_back(
            PolledDevice{
                mice_[index].fileDescriptor,
                true,
                index
            }
        );
    }

    std::vector<pollfd> descriptors{};
    descriptors.reserve(devices.size());

    for (const PolledDevice& device : devices) {
        descriptors.push_back(
            pollfd{
                device.fileDescriptor,
                POLLIN,
                0
            }
        );
    }

    while (true) {
        const int readyCount{
            ::poll(
                descriptors.data(),
                static_cast<nfds_t>(descriptors.size()),
                0
            )
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

            const PolledDevice& device{devices[index]};

            if (device.mouse) {
                readMouseEvents(mice_[device.index]);
            } else {
                readKeyboardEvents(keyboards_[device.index]);
            }

            eventRead = true;
        }

        if (!eventRead) {
            break;
        }

        for (pollfd& descriptor : descriptors) {
            descriptor.revents = 0;
        }
    }

    synchronizeKeyboardState();
    clampMousePosition();

    const bool controlDown{
        down(Scancode::LeftControl) ||
        down(Scancode::RightControl)
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

void TUIInput::setKeyboardLayout(KeyboardLayout layout) noexcept {
    if (keyboardLayout_ == layout) {
        return;
    }

    keyboardLayout_ = layout;

    // A layout change invalidates cached key states because the same physical
    // key can now map to a different logical Scancode.
    pressed_.fill(false);
    repeated_.fill(false);
    released_.fill(false);
    down_.fill(false);

    if (active_) {
        for (KeyboardDevice& keyboard : keyboards_) {
            resynchronizeKeyboard(keyboard);
        }
    }
}

KeyboardLayout TUIInput::keyboardLayout() const noexcept {
    return keyboardLayout_;
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

std::size_t TUIInput::mouseCount() const noexcept {
    return mice_.size();
}

void TUIInput::setMouseBounds(int width, int height) noexcept {
    mouse_.width = std::max(width, 1);
    mouse_.height = std::max(height, 1);
    clampMousePosition();
}

void TUIInput::setMousePosition(int x, int y) noexcept {
    mouse_.x = x;
    mouse_.y = y;
    mouse_.residualX = 0.0f;
    mouse_.residualY = 0.0f;
    clampMousePosition();
}

void TUIInput::setMouseSensitivity(
    float sensitivity
) noexcept {
    mouse_.sensitivity = std::clamp(
        sensitivity,
        0.0f,
        10.0f
    );
}

float TUIInput::mouseSensitivity() const noexcept {
    return mouse_.sensitivity;
}

int TUIInput::mouseX() const noexcept {
    return mouse_.x;
}

int TUIInput::mouseY() const noexcept {
    return mouse_.y;
}

int TUIInput::mouseDeltaX() const noexcept {
    return mouse_.deltaX;
}

int TUIInput::mouseDeltaY() const noexcept {
    return mouse_.deltaY;
}

int TUIInput::mouseWheelDelta() const noexcept {
    return mouse_.wheelDelta;
}

bool TUIInput::mousePressed(
    MouseButton button
) const noexcept {
    const std::size_t index{
        static_cast<std::size_t>(button)
    };

    return index < mouse_.pressed.size() &&
           mouse_.pressed[index];
}

bool TUIInput::mouseReleased(
    MouseButton button
) const noexcept {
    const std::size_t index{
        static_cast<std::size_t>(button)
    };

    return index < mouse_.released.size() &&
           mouse_.released[index];
}

bool TUIInput::mouseDown(
    MouseButton button
) const noexcept {
    const std::size_t index{
        static_cast<std::size_t>(button)
    };

    return index < mouse_.down.size() &&
           mouse_.down[index];
}

bool TUIInput::mouseLeftDown() const noexcept {
    return mouseDown(MouseButton::Left);
}

bool TUIInput::mouseRightDown() const noexcept {
    return mouseDown(MouseButton::Right);
}

bool TUIInput::mouseMiddleDown() const noexcept {
    return mouseDown(MouseButton::Middle);
}

bool TUIInput::mouseLeftPressed() const noexcept {
    return mousePressed(MouseButton::Left);
}

bool TUIInput::mouseRightPressed() const noexcept {
    return mousePressed(MouseButton::Right);
}

bool TUIInput::mouseMiddlePressed() const noexcept {
    return mousePressed(MouseButton::Middle);
}

bool TUIInput::mouseLeftReleased() const noexcept {
    return mouseReleased(MouseButton::Left);
}

bool TUIInput::mouseRightReleased() const noexcept {
    return mouseReleased(MouseButton::Right);
}

bool TUIInput::mouseMiddleReleased() const noexcept {
    return mouseReleased(MouseButton::Middle);
}

bool TUIInput::mouseBackDown() const noexcept {
    return mouseDown(MouseButton::Back);
}

bool TUIInput::mouseForwardDown() const noexcept {
    return mouseDown(MouseButton::Forward);
}

bool TUIInput::mouseBackPressed() const noexcept {
    return mousePressed(MouseButton::Back);
}

bool TUIInput::mouseForwardPressed() const noexcept {
    return mousePressed(MouseButton::Forward);
}

bool TUIInput::mouseBackReleased() const noexcept {
    return mouseReleased(MouseButton::Back);
}

bool TUIInput::mouseForwardReleased() const noexcept {
    return mouseReleased(MouseButton::Forward);
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

void TUIInput::discoverDevices() {
    namespace fs = std::filesystem;

    const fs::path inputDirectory{"/dev/input"};
    std::error_code error{};

    if (!fs::exists(inputDirectory, error)) {
        throw std::runtime_error(
            "TUIInput::discoverDevices(): /dev/input does not exist"
        );
    }

    std::vector<fs::path> candidates{};

    for (const fs::directory_entry& entry :
         fs::directory_iterator(inputDirectory, error)) {
        if (error) {
            break;
        }

        const std::string filename{
            entry.path().filename().string()
        };

        if (filename.rfind("event", 0U) == 0U) {
            candidates.push_back(entry.path());
        }
    }

    if (error) {
        throw std::runtime_error(
            "TUIInput::discoverDevices(): cannot enumerate /dev/input"
        );
    }

    std::sort(candidates.begin(), candidates.end());

    bool permissionDenied{false};

    for (const fs::path& path : candidates) {
        const int probeDescriptor{
            ::open(
                path.c_str(),
                O_RDONLY | O_NONBLOCK | O_CLOEXEC
            )
        };

        if (probeDescriptor == -1) {
            if (errno == EACCES || errno == EPERM) {
                permissionDenied = true;
            }

            continue;
        }

        const bool keyboard{isKeyboard(probeDescriptor)};
        const bool mouse{isMouse(probeDescriptor)};
        const std::string name{deviceName(probeDescriptor)};

        ::close(probeDescriptor);

        if (keyboard) {
            const int fileDescriptor{
                ::open(
                    path.c_str(),
                    O_RDONLY | O_NONBLOCK | O_CLOEXEC
                )
            };

            if (fileDescriptor != -1) {
                keyboards_.push_back(
                    KeyboardDevice{
                        fileDescriptor,
                        path.string(),
                        name,
                        true
                    }
                );
            }
        }

        if (mouse) {
            const int fileDescriptor{
                ::open(
                    path.c_str(),
                    O_RDONLY | O_NONBLOCK | O_CLOEXEC
                )
            };

            if (fileDescriptor != -1) {
                mice_.push_back(
                    MouseDevice{
                        fileDescriptor,
                        path.string(),
                        name,
                        true
                    }
                );
            }
        }
    }

    if (keyboards_.empty()) {
        if (permissionDenied) {
            throw std::runtime_error(
                "TUIInput::discoverDevices(): no readable keyboard found; "
                "run Hexodus with sufficient access to /dev/input/event*"
            );
        }

        throw std::runtime_error(
            "TUIInput::discoverDevices(): no evdev keyboard found"
        );
    }

    for (KeyboardDevice& keyboard : keyboards_) {
        resynchronizeKeyboard(keyboard);
    }

    for (MouseDevice& mouse : mice_) {
        resynchronizeMouse(mouse);
    }
}

void TUIInput::closeDevices() noexcept {
    for (KeyboardDevice& keyboard : keyboards_) {
        if (keyboard.fileDescriptor != -1) {
            ::close(keyboard.fileDescriptor);
            keyboard.fileDescriptor = -1;
        }
    }

    for (MouseDevice& mouse : mice_) {
        if (mouse.fileDescriptor != -1) {
            ::close(mouse.fileDescriptor);
            mouse.fileDescriptor = -1;
        }
    }

    keyboards_.clear();
    mice_.clear();
}

void TUIInput::clearFrameStates() noexcept {
    pressed_.fill(false);
    repeated_.fill(false);
    released_.fill(false);

    mouse_.deltaX = 0;
    mouse_.deltaY = 0;
    mouse_.wheelDelta = 0;

    mouse_.pressed.fill(false);
    mouse_.released.fill(false);
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

void TUIInput::readMouseEvents(MouseDevice& mouse) {
    std::array<input_event, 64> events{};

    while (true) {
        const ssize_t byteCount{
            ::read(
                mouse.fileDescriptor,
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
                    "TUIInput::readMouseEvents(): read failed for " +
                    mouse.path
                )
            );
        }

        if (byteCount == 0) {
            return;
        }

        if (
            static_cast<std::size_t>(byteCount) %
            sizeof(input_event) != 0U
        ) {
            throw std::runtime_error(
                "TUIInput::readMouseEvents(): incomplete evdev event"
            );
        }

        const std::size_t eventCount{
            static_cast<std::size_t>(byteCount) /
            sizeof(input_event)
        };

        for (std::size_t index{}; index < eventCount; ++index) {
            const input_event& event{events[index]};

            if (event.type == EV_SYN) {
                if (event.code == SYN_DROPPED) {
                    mouse.synchronized = false;
                } else if (
                    event.code == SYN_REPORT &&
                    !mouse.synchronized
                ) {
                    resynchronizeMouse(mouse);
                    mouse.synchronized = true;
                }

                continue;
            }

            if (!mouse.synchronized) {
                continue;
            }

            if (event.type == EV_REL) {
                switch (event.code) {
                    case REL_X: {
                        mouse_.residualX +=
                            static_cast<float>(event.value) *
                            mouse_.sensitivity;

                        const int movement{
                            static_cast<int>(
                                std::trunc(mouse_.residualX)
                            )
                        };

                        mouse_.residualX -=
                            static_cast<float>(movement);

                        mouse_.deltaX += movement;
                        mouse_.x += movement;
                        break;
                    }

                    case REL_Y: {
                        mouse_.residualY +=
                            static_cast<float>(event.value) *
                            mouse_.sensitivity;

                        const int movement{
                            static_cast<int>(
                                std::trunc(mouse_.residualY)
                            )
                        };

                        mouse_.residualY -=
                            static_cast<float>(movement);

                        mouse_.deltaY += movement;
                        mouse_.y += movement;
                        break;
                    }

                    case REL_WHEEL:
                        mouse_.wheelDelta += event.value;
                        break;

                    default:
                        break;
                }
            } else if (event.type == EV_KEY) {
                registerMouseButton(event.code, event.value);
            }
        }

        if (static_cast<std::size_t>(byteCount) < sizeof(events)) {
            return;
        }
    }
}

void TUIInput::resynchronizeKeyboard(KeyboardDevice& keyboard) {
    // Validate that this device can answer EVIOCGKEY, then recompute the
    // global state from every keyboard. This avoids one keyboard clearing
    // keys that are currently held on another keyboard.
    std::array<unsigned long, wordCountForBit(KEY_MAX)> keyState{};

    if (::ioctl(
        keyboard.fileDescriptor,
        EVIOCGKEY(sizeof(keyState)),
        keyState.data()
    ) == -1) {
        return;
    }

    synchronizeKeyboardState();
}

void TUIInput::synchronizeKeyboardState() noexcept {
    std::array<bool, scancodeCount_> combined{};

    for (const KeyboardDevice& keyboard : keyboards_) {
        std::array<unsigned long, wordCountForBit(KEY_MAX)> keyState{};

        if (::ioctl(
            keyboard.fileDescriptor,
            EVIOCGKEY(sizeof(keyState)),
            keyState.data()
        ) == -1) {
            continue;
        }

        for (std::uint16_t code{}; code <= KEY_MAX; ++code) {
            if (!bitIsSet(keyState.data(), code)) {
                continue;
            }

            const Scancode scancode{mapLinuxCode(code)};
            if (scancode == Scancode::Unknown) {
                continue;
            }

            const std::size_t index{static_cast<std::size_t>(scancode)};
            if (index < combined.size()) {
                combined[index] = true;
            }
        }
    }

    for (std::size_t index{}; index < combined.size(); ++index) {
        if (combined[index] && !down_[index]) {
            pressed_[index] = true;
        } else if (!combined[index] && down_[index]) {
            released_[index] = true;
        }

        down_[index] = combined[index];
    }
}

void TUIInput::resynchronizeMouse(MouseDevice& mouse) {
    std::array<unsigned long, wordCountForBit(KEY_MAX)> keyState{};

    if (::ioctl(
        mouse.fileDescriptor,
        EVIOCGKEY(sizeof(keyState)),
        keyState.data()
    ) == -1) {
        return;
    }

    const std::array<std::uint16_t, 5> linuxButtons{
        BTN_LEFT,
        BTN_RIGHT,
        BTN_MIDDLE,
        BTN_SIDE,
        BTN_EXTRA
    };

    for (const std::uint16_t linuxButton : linuxButtons) {
        const MouseButton button{
            fromLinuxMouseButton(linuxButton)
        };

        if (button == MouseButton::Count) {
            continue;
        }

        mouse_.down[
            static_cast<std::size_t>(button)
        ] = bitIsSet(
            keyState.data(),
            linuxButton
        );
    }
}

void TUIInput::registerMouseButton(
    std::uint16_t linuxCode,
    std::int32_t value
) noexcept {
    const MouseButton button{
        fromLinuxMouseButton(linuxCode)
    };

    if (button == MouseButton::Count) {
        return;
    }

    const std::size_t index{
        static_cast<std::size_t>(button)
    };

    if (value == 0) {
        if (mouse_.down[index]) {
            mouse_.released[index] = true;
        }

        mouse_.down[index] = false;
    } else if (value == 1) {
        if (!mouse_.down[index]) {
            mouse_.pressed[index] = true;
        }

        mouse_.down[index] = true;
    }
}

void TUIInput::clampMousePosition() noexcept {
    const int maximumX{
        std::max(mouse_.width - 1, 0)
    };

    const int maximumY{
        std::max(mouse_.height - 1, 0)
    };

    const int previousX{mouse_.x};
    const int previousY{mouse_.y};

    mouse_.x = std::clamp(
        mouse_.x,
        0,
        maximumX
    );

    mouse_.y = std::clamp(
        mouse_.y,
        0,
        maximumY
    );

    if (mouse_.x != previousX) {
        mouse_.residualX = 0.0f;
    }

    if (mouse_.y != previousY) {
        mouse_.residualY = 0.0f;
    }
}

void TUIInput::registerEvent(
    std::uint16_t linuxCode,
    std::int32_t value
) noexcept {
    const Scancode scancode{mapLinuxCode(linuxCode)};

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

bool TUIInput::isMouse(int fileDescriptor) noexcept {
    std::array<unsigned long, wordCountForBit(EV_MAX)> eventBits{};

    if (::ioctl(
        fileDescriptor,
        EVIOCGBIT(0, sizeof(eventBits)),
        eventBits.data()
    ) == -1) {
        return false;
    }

    if (
        !bitIsSet(eventBits.data(), EV_KEY) ||
        !bitIsSet(eventBits.data(), EV_REL)
    ) {
        return false;
    }

    std::array<unsigned long, wordCountForBit(REL_MAX)> relativeBits{};

    if (::ioctl(
        fileDescriptor,
        EVIOCGBIT(EV_REL, sizeof(relativeBits)),
        relativeBits.data()
    ) == -1) {
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

    return bitIsSet(relativeBits.data(), REL_X) &&
           bitIsSet(relativeBits.data(), REL_Y) &&
           bitIsSet(keyBits.data(), BTN_LEFT);
}


std::string TUIInput::deviceName(int fileDescriptor) {
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

MouseButton TUIInput::fromLinuxMouseButton(
    std::uint16_t linuxCode
) noexcept {
    switch (linuxCode) {
        case BTN_LEFT:
            return MouseButton::Left;

        case BTN_RIGHT:
            return MouseButton::Right;

        case BTN_MIDDLE:
            return MouseButton::Middle;

        case BTN_SIDE:
        case BTN_BACK:
            return MouseButton::Back;

        case BTN_EXTRA:
        case BTN_FORWARD:
            return MouseButton::Forward;

        default:
            return MouseButton::Count;
    }
}

Scancode TUIInput::mapLinuxCode(std::uint16_t linuxCode) const noexcept {
    switch (keyboardLayout_) {
        case KeyboardLayout::Azerty:
            return fromLinuxCodeAzerty(linuxCode);
        case KeyboardLayout::Qwerty:
        case KeyboardLayout::Physical:
        default:
            return fromLinuxCodePhysical(linuxCode);
    }
}

Scancode TUIInput::fromLinuxCodeAzerty(std::uint16_t linuxCode) noexcept {
    // Linux evdev codes describe physical key positions. These remaps make
    // Scancode::KeyA..KeyZ represent the visible letters on a standard
    // French AZERTY keyboard.
    switch (linuxCode) {
        case KEY_Q: return Scancode::KeyA;
        case KEY_W: return Scancode::KeyZ;
        case KEY_A: return Scancode::KeyQ;
        case KEY_Z: return Scancode::KeyW;
        case KEY_SEMICOLON: return Scancode::KeyM;

        // On AZERTY the physical US 'M' position is punctuation, not letter M.
        case KEY_M: return Scancode::Comma;

        default:
            return fromLinuxCodePhysical(linuxCode);
    }
}

Scancode TUIInput::fromLinuxCodePhysical(std::uint16_t linuxCode) noexcept {
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

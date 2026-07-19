#include "../include/TUIInput.hpp"

#include <cerrno>
#include <charconv>
#include <poll.h>
#include <stdexcept>
#include <string_view>
#include <unistd.h>

namespace TUI {
namespace {

bool parseInteger(std::string_view text, int& value) {
    if (text.empty()) {
        return false;
    }

    const char* const begin{text.data()};
    const char* const end{begin + text.size()};
    const auto result{std::from_chars(begin, end, value)};

    return result.ec == std::errc{} && result.ptr == end;
}

} // namespace

TUIInput::~TUIInput() {
    restore();
}

void TUIInput::activate() { if (active_) { return; }

    if (!::isatty(STDIN_FILENO)) {
        throw std::runtime_error("TUIInput::activate(): stdin is not a terminal");
    }

    if (::tcgetattr(STDIN_FILENO, &originalTermios_) == -1) {
        throw std::runtime_error("TUIInput::activate(): tcgetattr failed");
    }

    termios raw{originalTermios_};

    raw.c_iflag &= static_cast<tcflag_t>(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
    raw.c_cflag |= CS8;
    raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON | IEXTEN | ISIG));

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        throw std::runtime_error("TUIInput::activate(): tcsetattr failed");
    }

    pressed_.fill(false);
    down_.fill(false);
    pending_.clear();
    pendingEscapeSince_.reset();
    quitRequested_ = false;
    active_ = true;

}


void TUIInput::restore() noexcept {
    if (!active_) {
        return;
    }

    ::tcsetattr(
        STDIN_FILENO,
        TCSAFLUSH,
        &originalTermios_
    );

    pending_.clear();
    pendingEscapeSince_.reset();
    pressed_.fill(false);
    down_.fill(false);
    active_ = false;
}

void TUIInput::resetFrameState() noexcept {
    pressed_.fill(false);
    down_.fill(false);
}

void TUIInput::pollEvents() {
    if (!active_) {
        throw std::logic_error(
            "TUIInput::pollEvents(): input is not active"
        );
    }

    resetFrameState();
    readAvailableInput();
    parsePendingInput();
}

void TUIInput::readAvailableInput() {
    pollfd descriptor{
        STDIN_FILENO,
        POLLIN,
        0
    };

    while (true) {
        descriptor.revents = 0;

        const int pollResult{
            ::poll(&descriptor, 1, 0)
        };

        if (pollResult < 0) {
            if (errno == EINTR) {
                continue;
            }

            throw std::runtime_error(
                "TUIInput::pollEvents(): poll failed"
            );
        }

        if (
            pollResult == 0 ||
            (descriptor.revents & POLLIN) == 0
        ) {
            break;
        }

        char buffer[256];

        const ssize_t count{
            ::read(
                STDIN_FILENO,
                buffer,
                sizeof(buffer)
            )
        };

        if (count > 0) {
            pending_.append(
                buffer,
                static_cast<std::size_t>(count)
            );
            continue;
        }

        if (
            count < 0 &&
            (errno == EAGAIN || errno == EWOULDBLOCK)
        ) {
            break;
        }

        if (count < 0 && errno == EINTR) {
            continue;
        }

        break;
    }
}

void TUIInput::parsePendingInput() {
    std::size_t offset{0};

    while (offset < pending_.size()) {
        const unsigned char byte{
            static_cast<unsigned char>(
                pending_[offset]
            )
        };

        if (byte != 0x1B) {
            registerByte(byte);
            ++offset;
            continue;
        }

        const std::size_t consumed{
            parseEscapeSequence(offset)
        };

        if (consumed == 0U) {
            break;
        }

        offset += consumed;
    }

    if (offset != 0U) {
        pending_.erase(0, offset);
    }

    // ESC is also the first byte of arrows and function keys.
    // Wait briefly so a fragmented sequence cannot close the program.
    if (
        pending_.size() == 1U &&
        static_cast<unsigned char>(pending_[0]) == 0x1B
    ) {
        const auto now{Clock::now()};

        if (!pendingEscapeSince_) {
            pendingEscapeSince_ = now;
            return;
        }

        if (now - *pendingEscapeSince_ >= escapeDelay_) {
            registerKey(Key::Escape);
            pending_.clear();
            pendingEscapeSince_.reset();
        }

        return;
    }

    if (pending_.empty()) {
        pendingEscapeSince_.reset();
    }
}

std::size_t TUIInput::parseEscapeSequence(
    std::size_t offset
) {
    if (offset + 1U >= pending_.size()) {
        return 0U;
    }

    const char second{pending_[offset + 1U]};

    if (second == '[') {
        return parseCsiSequence(offset);
    }

    if (second == 'O') {
        if (offset + 2U >= pending_.size()) {
            return 0U;
        }

        switch (pending_[offset + 2U]) {
            case 'P': registerKey(Key::F1); break;
            case 'Q': registerKey(Key::F2); break;
            case 'R': registerKey(Key::F3); break;
            case 'S': registerKey(Key::F4); break;
            case 'H': registerKey(Key::Home); break;
            case 'F': registerKey(Key::End); break;
            default: break;
        }

        return 3U;
    }

    // Alt+key: report the key itself and consume the ESC prefix.
    registerByte(
        static_cast<unsigned char>(
            pending_[offset + 1U]
        )
    );

    return 2U;
}

std::size_t TUIInput::parseCsiSequence(
    std::size_t offset
) {
    std::size_t end{offset + 2U};

    while (end < pending_.size()) {
        const unsigned char value{
            static_cast<unsigned char>(pending_[end])
        };

        if (value >= 0x40U && value <= 0x7EU) {
            break;
        }

        ++end;
    }

    if (end >= pending_.size()) {
        return 0U;
    }

    const char final{pending_[end]};
    const std::string_view parameters{
        pending_.data() + offset + 2U,
        end - (offset + 2U)
    };

    switch (final) {
        case 'A': registerKey(Key::ArrowUp); break;
        case 'B': registerKey(Key::ArrowDown); break;
        case 'C': registerKey(Key::ArrowRight); break;
        case 'D': registerKey(Key::ArrowLeft); break;
        case 'H': registerKey(Key::Home); break;
        case 'F': registerKey(Key::End); break;

        case '~': {
            const std::size_t separator{
                parameters.find(';')
            };

            const std::string_view first{
                parameters.substr(0, separator)
            };

            int number{0};

            if (parseInteger(first, number)) {
                registerKey(
                    keyFromCsiNumber(number)
                );
            }

            break;
        }

        default:
            // Unknown CSI sequences are consumed safely.
            break;
    }

    return end - offset + 1U;
}

void TUIInput::registerByte(unsigned char byte) {
    // ISIG is disabled in raw mode, so Ctrl+C arrives as byte 3.
    if (byte == 3U) {
        quitRequested_ = true;
        return;
    }

    registerKey(keyFromAscii(byte));
}

void TUIInput::registerKey(Key key) noexcept {
    if (key == Key::Unknown) {
        return;
    }

    const std::size_t index{
        static_cast<std::size_t>(key)
    };

    pressed_[index] = true;
    down_[index] = true;
}

bool TUIInput::keyPressed(Key key) const noexcept {
    const std::size_t index{
        static_cast<std::size_t>(key)
    };

    return index < keyCount_ && pressed_[index];
}

bool TUIInput::keyDown(Key key) const noexcept {
    const std::size_t index{
        static_cast<std::size_t>(key)
    };

    return index < keyCount_ && down_[index];
}

Key TUIInput::keyFromAscii(
    unsigned char byte
) noexcept {
    if (byte >= 'a' && byte <= 'z') {
        return static_cast<Key>(
            static_cast<int>(Key::A) +
            static_cast<int>(byte - 'a')
        );
    }

    if (byte >= 'A' && byte <= 'Z') {
        return static_cast<Key>(
            static_cast<int>(Key::A) +
            static_cast<int>(byte - 'A')
        );
    }

    if (byte >= '0' && byte <= '9') {
        return static_cast<Key>(
            static_cast<int>(Key::Num0) +
            static_cast<int>(byte - '0')
        );
    }

    switch (byte) {
        case '\r':
        case '\n':
            return Key::Enter;

        case '\t':
            return Key::Tab;

        case 0x7F:
        case 0x08:
            return Key::Backspace;

        case ' ':
            return Key::Space;

        case 0x1B:
            return Key::Escape;

        default:
            return Key::Unknown;
    }
}

Key TUIInput::keyFromCsiNumber(int number) noexcept {
    switch (number) {
        case 1:
        case 7:
            return Key::Home;

        case 2:
            return Key::Insert;

        case 3:
            return Key::Delete;

        case 4:
        case 8:
            return Key::End;

        case 5:
            return Key::PageUp;

        case 6:
            return Key::PageDown;

        case 11: return Key::F1;
        case 12: return Key::F2;
        case 13: return Key::F3;
        case 14: return Key::F4;
        case 15: return Key::F5;
        case 17: return Key::F6;
        case 18: return Key::F7;
        case 19: return Key::F8;
        case 20: return Key::F9;
        case 21: return Key::F10;
        case 23: return Key::F11;
        case 24: return Key::F12;

        default:
            return Key::Unknown;
    }
}

} // namespace TUI

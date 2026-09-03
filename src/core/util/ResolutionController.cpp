#include "../../include/util/ResolutionController.hpp"

#include <algorithm>

namespace TUI {

void ResolutionController::setMode(ResolutionMode mode) noexcept {
    mode_ = mode;
}

ResolutionMode ResolutionController::mode() const noexcept {
    return mode_;
}

void ResolutionController::setMinimum(RenderResolution resolution) {
    setMinimum(resolution.width, resolution.height);
}

void ResolutionController::setMinimum(int width, int height) {
    minimum_.width = std::max(width, 1);
    minimum_.height = std::max(height, 1);
}

const RenderResolution& ResolutionController::minimum() const noexcept {
    return minimum_;
}

void ResolutionController::setMaximum(RenderResolution resolution) {
    setMaximum(resolution.width, resolution.height);
}

void ResolutionController::setMaximum(int width, int height) {
    maximum_.width = std::max(width, 0);
    maximum_.height = std::max(height, 0);
}

void ResolutionController::clearMaximum() noexcept {
    maximum_ = RenderResolution{};
}

const RenderResolution& ResolutionController::maximum() const noexcept {
    return maximum_;
}

void ResolutionController::setFixed(RenderResolution resolution) {
    setFixed(resolution.width, resolution.height);
}

void ResolutionController::setFixed(int width, int height) {
    fixed_.width = std::max(width, 1);
    fixed_.height = std::max(height, 1);
}

const RenderResolution& ResolutionController::fixed() const noexcept {
    return fixed_;
}

RenderResolution ResolutionController::resolve(
    TerminalSize terminalSize,
    int cellPixelWidth,
    int cellPixelHeight
) const noexcept {
    cellPixelWidth = std::max(cellPixelWidth, 1);
    cellPixelHeight = std::max(cellPixelHeight, 1);

    const RenderResolution available{
        std::max(terminalSize.width, 0) * cellPixelWidth,
        std::max(terminalSize.height, 0) * cellPixelHeight
    };

    RenderResolution result{available};

    switch (mode_) {
    case ResolutionMode::Automatic:
        break;

    case ResolutionMode::Capped:
        if (maximum_.width > 0) {
            result.width = std::min(result.width, maximum_.width);
        }

        if (maximum_.height > 0) {
            result.height = std::min(result.height, maximum_.height);
        }
        break;

    case ResolutionMode::Fixed:
        result = fixed_;
        break;
    }

    return result;
}

bool ResolutionController::supported(
    TerminalSize terminalSize,
    int cellPixelWidth,
    int cellPixelHeight
) const noexcept {
    cellPixelWidth = std::max(cellPixelWidth, 1);
    cellPixelHeight = std::max(cellPixelHeight, 1);

    const RenderResolution available{
        std::max(terminalSize.width, 0) * cellPixelWidth,
        std::max(terminalSize.height, 0) * cellPixelHeight
    };

    if (
        available.width < minimum_.width ||
        available.height < minimum_.height
    ) {
        return false;
    }

    if (
        mode_ == ResolutionMode::Fixed &&
        (
            fixed_.width > available.width ||
            fixed_.height > available.height
        )
    ) {
        return false;
    }

    return true;
}

} // namespace TUI

#include "../include/TUIRenderer.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace TUI {

TUIRenderer::TUIRenderer()
    : framebuffer_{},
      graphicManager_{},
      scenes_{},
      activeScene_{nullptr} {}

int TUIRenderer::colorDistanceSquared(const Pixel& lhs, const Pixel& rhs) {
    const int dr = static_cast<int>(lhs.r) - static_cast<int>(rhs.r);
    const int dg = static_cast<int>(lhs.g) - static_cast<int>(rhs.g);
    const int db = static_cast<int>(lhs.b) - static_cast<int>(rhs.b);

    return dr * dr + dg * dg + db * db;
}

RGBPanel TUIRenderer::toRGBPanel(const Pixel& pixel) {
    return RGBPanel{pixel.r, pixel.g, pixel.b};
}

std::string_view TUIRenderer::glyphForMask(std::uint8_t mask) {
    switch (mask & 0x0F) {
        case 0x0: return QuadBlock::empty;
        case 0x1: return QuadBlock::tl;
        case 0x2: return QuadBlock::tr;
        case 0x3: return QuadBlock::tl_tr;
        case 0x4: return QuadBlock::bl;
        case 0x5: return QuadBlock::tl_bl;
        case 0x6: return QuadBlock::tr_bl;
        case 0x7: return QuadBlock::tl_tr_bl;
        case 0x8: return QuadBlock::br;
        case 0x9: return QuadBlock::tl_br;
        case 0xA: return QuadBlock::tr_br;
        case 0xB: return QuadBlock::tl_tr_br;
        case 0xC: return QuadBlock::bl_br;
        case 0xD: return QuadBlock::tl_bl_br;
        case 0xE: return QuadBlock::tr_bl_br;
        case 0xF: return QuadBlock::full;
        default:  return QuadBlock::empty;
    }
}

TUIRenderer::EncodedCell TUIRenderer::encodeQuad(
    const Pixel& topLeft,
    const Pixel& topRight,
    const Pixel& bottomLeft,
    const Pixel& bottomRight
) const {
    const std::array<Pixel, 4> pixels{
        topLeft,
        topRight,
        bottomLeft,
        bottomRight
    };

    // A terminal cell can display only two colors: one foreground and one
    // background. Pick the two most distant colors in the 2x2 source block,
    // then assign every source pixel to its nearest representative color.
    std::size_t backgroundIndex = 0;
    std::size_t foregroundIndex = 0;
    int greatestDistance = -1;

    for (std::size_t first = 0; first < pixels.size(); ++first) {
        for (std::size_t second = first + 1; second < pixels.size(); ++second) {
            const int distance = colorDistanceSquared(pixels[first], pixels[second]);

            if (distance > greatestDistance) {
                greatestDistance = distance;
                backgroundIndex = first;
                foregroundIndex = second;
            }
        }
    }

    const Pixel background = pixels[backgroundIndex];
    const Pixel foreground = pixels[foregroundIndex];

    // All four pixels are effectively the same color.
    if (greatestDistance <= 0) {
        return EncodedCell{
            QuadBlock::empty,
            toRGBPanel(background),
            toRGBPanel(background)
        };
    }

    std::uint8_t mask = 0;

    for (std::size_t index = 0; index < pixels.size(); ++index) {
        const int distanceToBackground =
            colorDistanceSquared(pixels[index], background);
        const int distanceToForeground =
            colorDistanceSquared(pixels[index], foreground);

        if (distanceToForeground < distanceToBackground) {
            mask |= static_cast<std::uint8_t>(1U << index);
        }
    }

    // Avoid emitting a full block when reversing foreground/background gives
    // the same picture with an empty cell, which is usually cheaper to draw.
    if (mask == 0x0F) {
        mask = 0x00;
        return EncodedCell{
            glyphForMask(mask),
            toRGBPanel(background),
            toRGBPanel(foreground)
        };
    }

    return EncodedCell{
        glyphForMask(mask),
        toRGBPanel(foreground),
        toRGBPanel(background)
    };
}

void TUIRenderer::renderScenes() {
    assert(
        activeScene_ != nullptr &&
        "TUIRenderer::renderScenes(): activeScene_ is null"
    );

    graphicManager_.update();

    // One terminal cell represents a 2x2 logical pixel block.
    const int logicalWidth = graphicManager_.getWidth() * 2;
    const int logicalHeight = graphicManager_.getHeigth() * 2;

    framebuffer_.assign(
        static_cast<std::size_t>(logicalWidth) *
            static_cast<std::size_t>(logicalHeight),
        Pixel{0, 0, 0, 255}
    );

    std::size_t activeSceneIndex = scenes_.size();

    for (std::size_t index = 0; index < scenes_.size(); ++index) {
        if (scenes_[index] == activeScene_) {
            activeSceneIndex = index;
            continue;
        }

        renderScene(index);
    }

    if (activeSceneIndex < scenes_.size()) {
        renderScene(activeSceneIndex);
    }
}

void TUIRenderer::renderScene(std::size_t index) {
    Scene* scene = scenes_.at(index);
    if (scene == nullptr) {
        return;
    }

    const int logicalHeight = graphicManager_.getHeigth() * 2;
    const int logicalWidth = graphicManager_.getWidth() * 2;

    const int sceneOriginX =
        (static_cast<int>(scene->Origin()[0]) * logicalWidth) / 100;
    const int sceneOriginY =
        (static_cast<int>(scene->Origin()[1]) * logicalHeight) / 100;
    const int sceneEndingX =
        (static_cast<int>(scene->Ending()[0]) * logicalWidth) / 100;
    const int sceneEndingY =
        (static_cast<int>(scene->Ending()[1]) * logicalHeight) / 100;

    const int sceneWidth = sceneEndingX - sceneOriginX;
    const int sceneHeight = sceneEndingY - sceneOriginY;

    if (sceneWidth <= 0 || sceneHeight <= 0) {
        return;
    }

    const std::vector<Pixel> sceneBuffer =
        scene->convertScene(logicalHeight, logicalWidth);

    const std::size_t expectedSize =
        static_cast<std::size_t>(sceneWidth) *
        static_cast<std::size_t>(sceneHeight);

    if (sceneBuffer.size() < expectedSize) {
        throw std::runtime_error(
            "TUIRenderer::renderScene(): scene buffer is too small"
        );
    }

    for (int y = 0; y < sceneHeight; ++y) {
        for (int x = 0; x < sceneWidth; ++x) {
            const int screenX = sceneOriginX + x;
            const int screenY = sceneOriginY + y;

            if (screenX < 0 || screenX >= logicalWidth ||
                screenY < 0 || screenY >= logicalHeight) {
                continue;
            }

            const Pixel source = sceneBuffer[
                static_cast<std::size_t>(y * sceneWidth + x)
            ];

            if (source.a == 0) {
                continue;
            }

            framebuffer_[
                static_cast<std::size_t>(screenY * logicalWidth + screenX)
            ] = source;
        }
    }
}

void TUIRenderer::InitScene(Scene* scene) {
    if (scene == nullptr) {
        throw std::invalid_argument(
            "TUIRenderer::InitScene(): scene is null"
        );
    }

    scenes_.push_back(scene);
    activeScene_ = scene;
}

void TUIRenderer::drawBuffer() {
    graphicManager_.TUICursorHome();
    renderScenes();

    const int terminalHeight = graphicManager_.getHeigth();
    const int terminalWidth = graphicManager_.getWidth();
    const int logicalWidth = terminalWidth * 2;

    RGBPanel lastForeground{};
    RGBPanel lastBackground{};
    bool hasLastColors = false;

    for (int cellY = 0; cellY < terminalHeight; ++cellY) {
        for (int cellX = 0; cellX < terminalWidth; ++cellX) {
            const int sourceX = cellX * 2;
            const int sourceY = cellY * 2;

            const Pixel& topLeft = framebuffer_[
                static_cast<std::size_t>(sourceY * logicalWidth + sourceX)
            ];
            const Pixel& topRight = framebuffer_[
                static_cast<std::size_t>(sourceY * logicalWidth + sourceX + 1)
            ];
            const Pixel& bottomLeft = framebuffer_[
                static_cast<std::size_t>((sourceY + 1) * logicalWidth + sourceX)
            ];
            const Pixel& bottomRight = framebuffer_[
                static_cast<std::size_t>((sourceY + 1) * logicalWidth + sourceX + 1)
            ];

            const EncodedCell encoded = encodeQuad(
                topLeft,
                topRight,
                bottomLeft,
                bottomRight
            );

            const bool sameColors =
                hasLastColors &&
                encoded.foreground.R_ == lastForeground.R_ &&
                encoded.foreground.G_ == lastForeground.G_ &&
                encoded.foreground.B_ == lastForeground.B_ &&
                encoded.background.R_ == lastBackground.R_ &&
                encoded.background.G_ == lastBackground.G_ &&
                encoded.background.B_ == lastBackground.B_;

            if (sameColors) {
                graphicManager_.addPixel(encoded.glyph);
            } else {
                graphicManager_.addPixel(
                    encoded.glyph,
                    encoded.foreground,
                    encoded.background
                );

                lastForeground = encoded.foreground;
                lastBackground = encoded.background;
                hasLastColors = true;
            }
        }
    }

    graphicManager_.execCommand();
}

void TUIRenderer::setActiveScene(Scene* scene) {
    if (scene == nullptr) {
        throw std::invalid_argument(
            "TUIRenderer::setActiveScene(): scene is null"
        );
    }

    activeScene_ = scene;
}

void TUIRenderer::activate() {
    graphicManager_.TUIEnterAltScreen();
    graphicManager_.TUIHideCursor();
    graphicManager_.TUIClear();
    graphicManager_.TUICursorHome();
}

void TUIRenderer::restore() {
    graphicManager_.TUIResetColors();
    graphicManager_.disableRawMode();
    graphicManager_.TUIShowCursor();
    graphicManager_.TUILeaveAltScreen();
}

} // namespace TUI

#include "../include/TUIRenderer.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string_view>

namespace TUI {

TUIRenderer::TUIRenderer()
    : framebuffer_{},
      pixelCommands_{},
      currentCells_{},
      previousCells_{},
      graphicManager_{},
      scenes_{},
      activeScene_{nullptr},
      requestedTerminalMode_{TerminalMode::Automatic},
      activeTerminalMode_{detectTerminalMode()} {}

bool TUIRenderer::EncodedCell::operator==( const EncodedCell& other) const noexcept {
    return glyph == other.glyph &&
           TUIRenderer::sameColor(foreground,other.foreground) &&
           TUIRenderer::sameColor(background, other.background);
}

bool TUIRenderer::sameColor(const RGBPanel& lhs, const RGBPanel& rhs) noexcept {
    return lhs.R_ == rhs.R_ && lhs.G_ == rhs.G_ && lhs.B_ == rhs.B_;
}

int TUIRenderer::horizontalPixelsPerCell() const noexcept {
    if (activeTerminalMode_ == TerminalMode::LinuxConsole) {
        return 1;
    }

    return 2;
}

int TUIRenderer::verticalPixelsPerCell() const noexcept {
    if (activeTerminalMode_ == TerminalMode::LinuxConsole) {
        return 1;
    }

    return 2;
}

TerminalMode TUIRenderer::detectTerminalMode() noexcept {
    const char* terminalEnvironment{std::getenv("TERM")};

    if (terminalEnvironment == nullptr) {
        return TerminalMode::UnicodeQuad;
    }

    const std::string_view terminal{terminalEnvironment};

    if (terminal == "linux" || terminal.starts_with("linux-")) {
        return TerminalMode::LinuxConsole;
    }

    return TerminalMode::UnicodeQuad;
}

int TUIRenderer::colorDistanceSquared(const Pixel& lhs, const Pixel& rhs) {
    const int redDifference{static_cast<int>(lhs.r) - static_cast<int>(rhs.r)};
    const int greenDifference{static_cast<int>(lhs.g) - static_cast<int>(rhs.g)};
    const int blueDifference{static_cast<int>(lhs.b) - static_cast<int>(rhs.b)};

    return redDifference * redDifference + greenDifference * greenDifference + blueDifference * blueDifference;
}

RGBPanel TUIRenderer::toRGBPanel(const Pixel& pixel) {
    return RGBPanel{pixel.r, pixel.g, pixel.b};
}

std::string_view TUIRenderer::glyphForMask(std::uint8_t mask) {
    switch (mask & 0x0F) {
        case 0x0:
            return QuadBlock::empty;

        case 0x1:
            return QuadBlock::tl;

        case 0x2:
            return QuadBlock::tr;

        case 0x3:
            return QuadBlock::tl_tr;

        case 0x4:
            return QuadBlock::bl;

        case 0x5:
            return QuadBlock::tl_bl;

        case 0x6:
            return QuadBlock::tr_bl;

        case 0x7:
            return QuadBlock::tl_tr_bl;

        case 0x8:
            return QuadBlock::br;

        case 0x9:
            return QuadBlock::tl_br;

        case 0xA:
            return QuadBlock::tr_br;

        case 0xB:
            return QuadBlock::tl_tr_br;

        case 0xC:
            return QuadBlock::bl_br;

        case 0xD:
            return QuadBlock::tl_bl_br;

        case 0xE:
            return QuadBlock::tr_bl_br;

        case 0xF:
            return QuadBlock::full;

        default:
            return QuadBlock::empty;
    }
}

TUIRenderer::EncodedCell TUIRenderer::encodeQuad(const Pixel& topLeft, const Pixel& topRight, const Pixel& bottomLeft, const Pixel& bottomRight) {
    const std::array<Pixel, 4> pixels{topLeft, topRight, bottomLeft, bottomRight};
    // Each partition and its complement describe the same image. Keep pixel 3
    // in the background: masks 0..7 cover all eight distinct partitions.
    // Minimize squared RGB error; alpha has already been composited upstream.
    int bestError{4 * 3 * 255 * 255 + 1};
    EncodedCell best{};
    for (unsigned mask{}; mask < 8; ++mask) {
        int sums[2][3]{};
        int counts[2]{};
        for (unsigned i{}; i < 4; ++i) {
            const unsigned group{(mask >> i) & 1U};
            ++counts[group];
            sums[group][0] += pixels[i].r;
            sums[group][1] += pixels[i].g;
            sums[group][2] += pixels[i].b;
        }
        RGBPanel colors[2]{};
        for (unsigned group{}; group < 2; ++group) {
            if (counts[group] == 0) continue;
            const int n{counts[group]};
            // Rounded centroid is optimal for integer RGB channels.
            colors[group] = RGBPanel{
                static_cast<unsigned char>((sums[group][0] + n / 2) / n),
                static_cast<unsigned char>((sums[group][1] + n / 2) / n),
                static_cast<unsigned char>((sums[group][2] + n / 2) / n)};
        }
        if (counts[1] == 0) colors[1] = colors[0];
        int error{};
        for (unsigned i{}; i < 4; ++i) {
            const auto color{colors[(mask >> i) & 1U]};
            const int dr{int(pixels[i].r) - color.R_};
            const int dg{int(pixels[i].g) - color.G_};
            const int db{int(pixels[i].b) - color.B_};
            error += dr * dr + dg * dg + db * db;
        }
        // Strict comparison makes ties deterministic and prefers solid cells.
        if (error < bestError) {
            bestError = error;
            best = EncodedCell{glyphForMask(mask), colors[1], colors[0]};
        }
    }
    return best;
}

void TUIRenderer::renderScenes() {
    assert(activeScene_ != nullptr && "TUIRenderer::renderScenes(): activeScene_ is null");

    graphicManager_.update();

    const int logicalWidth{graphicManager_.getWidth() * horizontalPixelsPerCell()};
    const int logicalHeight{graphicManager_.getHeigth() * verticalPixelsPerCell()};

    const std::size_t framebufferSize{static_cast<std::size_t>(logicalWidth) * static_cast<std::size_t>(logicalHeight)};

    if (framebuffer_.size() != framebufferSize) {
        framebuffer_.resize(framebufferSize);
    }

    std::fill(framebuffer_.begin(), framebuffer_.end(), Pixel{0, 0, 0, 255});

    std::size_t activeSceneIndex{scenes_.size()};

    for(std::size_t index{}; index < scenes_.size(); ++index) {
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

void TUIRenderer::renderScene(
    std::size_t index) {
    Scene* scene{scenes_.at(index)};

    if (scene == nullptr) {
        return;
    }

    const int logicalHeight{graphicManager_.getHeigth() * verticalPixelsPerCell()};
    const int logicalWidth{graphicManager_.getWidth() * horizontalPixelsPerCell()};

    const int sceneOriginX{(static_cast<int>(scene->Origin()[0]) * logicalWidth) / 100};
    const int sceneOriginY{(static_cast<int>(scene->Origin()[1]) * logicalHeight) / 100};
    const int sceneEndingX{(static_cast<int>(scene->Ending()[0]) * logicalWidth) / 100};
    const int sceneEndingY{(static_cast<int>(scene->Ending()[1]) * logicalHeight) / 100};

    const int sceneWidth{sceneEndingX - sceneOriginX};
    const int sceneHeight{sceneEndingY - sceneOriginY};

    if (sceneWidth <= 0 || sceneHeight <= 0) {
        return;
    }

    const std::vector<Pixel> sceneBuffer{scene->convertScene(logicalHeight, logicalWidth)};

    const std::size_t expectedSize{
        static_cast<std::size_t>(sceneWidth) *
        static_cast<std::size_t>(sceneHeight)
    };

    if (sceneBuffer.size() < expectedSize) {
        throw std::runtime_error("TUIRenderer::renderScene(): scene buffer is too small");
    }

    for(int y{}; y < sceneHeight; ++y) {
        for(int x{}; x < sceneWidth; ++x) {
            const int screenX{sceneOriginX + x};
            const int screenY{sceneOriginY + y};

            if (screenX < 0 || screenX >= logicalWidth || screenY < 0 || screenY >= logicalHeight) {
                continue;
            }

            const Pixel source{sceneBuffer[static_cast<std::size_t>(y * sceneWidth + x)]};

            if (source.a == 0) {
                continue;
            }

            framebuffer_[static_cast<std::size_t>(screenY * logicalWidth + screenX)] = source;
        }
    }
}

void TUIRenderer::applyPixelCommands() {
    const int width{graphicManager_.getWidth() * horizontalPixelsPerCell()};
    const int height{graphicManager_.getHeigth() * verticalPixelsPerCell()};

    for(const PixelCommand& command : pixelCommands_) {
        if (command.x < 0 || command.x >= width || command.y < 0 || command.y >= height) {
            continue;
        }

        const std::size_t pixelIndex{static_cast<std::size_t>(command.y * width + command.x)};

        if (command.mode == PixelDrawingMode::Contrasted) {
            if (activeTerminalMode_ == TerminalMode::UnicodeQuad) {
                framebuffer_[pixelIndex] = Pixel{255, 255, 255, 255};
            } else {
                const Pixel& background{framebuffer_[pixelIndex]};

                const int luminance{(
                        299 * static_cast<int>(background.r) +
                        587 * static_cast<int>(background.g) +
                        114 * static_cast<int>(background.b)
                    ) / 1000
                };

                framebuffer_[pixelIndex] = luminance >= 128 ? Pixel{0, 0, 0, 255} : Pixel{255, 255, 255, 255};
            }
        } else {
            framebuffer_[pixelIndex] = command.color;
        }
    }

    pixelCommands_.clear();
}

void TUIRenderer::encodeFramebuffer(int terminalWidth, int terminalHeight) {
    if (activeTerminalMode_ == TerminalMode::LinuxConsole) {
        encodeLinuxConsoleFramebuffer(terminalWidth, terminalHeight);
        return;
    }

    encodeUnicodeFramebuffer(terminalWidth, terminalHeight);
}

void TUIRenderer::encodeUnicodeFramebuffer(int terminalWidth, int terminalHeight) {
    const int logicalWidth{terminalWidth * 2};

    const std::size_t cellCount{
        static_cast<std::size_t>(terminalWidth) *
        static_cast<std::size_t>(terminalHeight)
    };

    currentCells_.resize(cellCount);

    for(int cellY{}; cellY < terminalHeight; ++cellY) {
        for(int cellX{}; cellX < terminalWidth; ++cellX) {
            const int sourceX{cellX * 2};
            const int sourceY{cellY * 2};

            const Pixel& topLeft{framebuffer_[static_cast<std::size_t>(sourceY * logicalWidth + sourceX)]};
            const Pixel& topRight{framebuffer_[static_cast<std::size_t>(sourceY * logicalWidth + sourceX + 1)]};
            const Pixel& bottomLeft{framebuffer_[static_cast<std::size_t>((sourceY + 1) * logicalWidth + sourceX)]};
            const Pixel& bottomRight{framebuffer_[static_cast<std::size_t>((sourceY + 1) * logicalWidth + sourceX + 1)]};

            currentCells_[static_cast<std::size_t>(cellY * terminalWidth + cellX)] = 
                encodeQuad(topLeft, topRight, bottomLeft, bottomRight);
        }
    }
}

void TUIRenderer::encodeLinuxConsoleFramebuffer(int terminalWidth,int terminalHeight) { const std::size_t cellCount{
    static_cast<std::size_t>(terminalWidth) * static_cast<std::size_t>(terminalHeight)};

    currentCells_.resize(cellCount);

    for(int cellY{}; cellY < terminalHeight; ++cellY) {
        for(int cellX{}; cellX < terminalWidth; ++cellX) {
            const Pixel& pixel{
                framebuffer_[static_cast<std::size_t>(cellY * terminalWidth + cellX)]
            };

            const RGBPanel color{toRGBPanel(pixel)};

            currentCells_[static_cast<std::size_t>(cellY * terminalWidth +cellX)] = 
                EncodedCell{" ", color, color};
        }
    }
}

void TUIRenderer::appendCell(const EncodedCell& cell, RGBPanel& lastForeground, RGBPanel& lastBackground, bool& hasLastColors) {
    const bool sameColors{
        hasLastColors &&
        sameColor(cell.foreground, lastForeground) &&
        sameColor(cell.background, lastBackground)
    };

    if (sameColors) {
        graphicManager_.addPixel(cell.glyph);
        return;
    }

    graphicManager_.addPixel(
        cell.glyph,
        cell.foreground,
        cell.background
    );

    lastForeground = cell.foreground;
    lastBackground = cell.background;
    hasLastColors = true;
}

void TUIRenderer::drawFullFrame(int terminalWidth, int terminalHeight) {

    graphicManager_.addCursorHome();

    RGBPanel lastForeground{};
    RGBPanel lastBackground{};
    bool hasLastColors{false};

    for(const EncodedCell& cell : currentCells_) {
        appendCell(
            cell,
            lastForeground,
            lastBackground,
            hasLastColors
        );
    }
}

void TUIRenderer::drawDifferentialFrame(int terminalWidth, int terminalHeight) {
    RGBPanel lastForeground{};
    RGBPanel lastBackground{};
    bool hasLastColors{false};

    for(int y{}; y < terminalHeight; ++y) {
        int x{};

        while (x < terminalWidth) {
            while (x < terminalWidth) {
                const std::size_t index{static_cast<std::size_t>(y * terminalWidth + x)};

                if (currentCells_[index] != previousCells_[index]) {
                    break;
                }

                ++x;
            }

            if (x >= terminalWidth) {
                break;
            }

            const int runStart{x};

            while (x < terminalWidth) {
                const std::size_t index{static_cast<std::size_t>(y * terminalWidth + x)};

                if (currentCells_[index] == previousCells_[index]) {
                    break;
                }

                ++x;
            }

            graphicManager_.addCursorMove(y + 1, runStart + 1);

            for(int runX{runStart}; runX < x; ++runX) {
                const std::size_t index{static_cast<std::size_t>(y * terminalWidth + runX)};

                appendCell(currentCells_[index], lastForeground, lastBackground, hasLastColors);
            }
        }
    }
}

void TUIRenderer::drawPixel(int x, int y,
    Pixel color) {
    pixelCommands_.push_back(
        PixelCommand{x, y, color, PixelDrawingMode::Normal}
    );
}

void TUIRenderer::drawContrastedPixel(int x, int y) {
    pixelCommands_.push_back(
        PixelCommand{x, y, Pixel{255, 255, 255, 255}, PixelDrawingMode::Contrasted}
    );
}

int TUIRenderer::logicalWidth() {
    graphicManager_.update();

    return graphicManager_.getWidth() * horizontalPixelsPerCell();
}

int TUIRenderer::logicalHeight() {
    graphicManager_.update();

    return graphicManager_.getHeigth() * verticalPixelsPerCell();
}

void TUIRenderer::drawBuffer() {
    renderScenes();
    applyPixelCommands();

    const int terminalHeight{graphicManager_.getHeigth()};
    const int terminalWidth{graphicManager_.getWidth()};

    if (terminalHeight <= 0 || terminalWidth <= 0) {
        return;
    }

    const bool resized{terminalWidth != previousTerminalWidth_ || terminalHeight != previousTerminalHeight_};

    if (resized) {
        previousTerminalWidth_ = terminalWidth;
        previousTerminalHeight_ = terminalHeight;
        previousCells_.clear();
        forceFullRedraw_ = true;

        graphicManager_.reserveCommand(static_cast<std::size_t>(terminalWidth) * static_cast<std::size_t>(terminalHeight) * 24U);
    }

    encodeFramebuffer(terminalWidth, terminalHeight);

    std::size_t changedCells{currentCells_.size()};

    if (!forceFullRedraw_ && previousCells_.size() == currentCells_.size()) {
        changedCells = 0;

        for(std::size_t index{}; index < currentCells_.size(); ++index) {
            if (
                currentCells_[index] !=
                previousCells_[index]
            ) {
                ++changedCells;
            }
        }
    }

    if (!forceFullRedraw_ &&changedCells == 0U) {
        previousCells_.swap(currentCells_);
        return;
    }

    const float changedRatio{currentCells_.empty() ? 1.0f : static_cast<float>(changedCells) /  static_cast<float>(currentCells_.size())};

    const bool fullRedraw{
        forceFullRedraw_ || previousCells_.size() !=
        currentCells_.size() || changedRatio >= fullRedrawThreshold_
    };

    graphicManager_.beginFrame();

    if (fullRedraw) {
        drawFullFrame(terminalWidth, terminalHeight);
    } else {
        drawDifferentialFrame(terminalWidth, terminalHeight);
    }

    graphicManager_.endFrame();
    graphicManager_.execCommand();

    previousCells_.swap(currentCells_);
    forceFullRedraw_ = false;
}

void TUIRenderer::InitScene(
    Scene* scene) {
    if (scene == nullptr) {
        throw std::invalid_argument("TUIRenderer::InitScene(): scene is null");
    }

    scenes_.push_back(scene);
    activeScene_ = scene;
    invalidate();
}

void TUIRenderer::setActiveScene(Scene* scene) {
    if (scene == nullptr) {
        throw std::invalid_argument("TUIRenderer::setActiveScene(): scene is null");
    }

    activeScene_ = scene;
    invalidate();
}

void TUIRenderer::setTerminalMode(TerminalMode mode) {
    requestedTerminalMode_ = mode;

    if (mode == TerminalMode::Automatic) {
        activeTerminalMode_ = detectTerminalMode();
    } else {
        activeTerminalMode_ = mode;
    }

    framebuffer_.clear();
    currentCells_.clear();
    previousCells_.clear();

    previousTerminalWidth_ = 0;
    previousTerminalHeight_ = 0;

    invalidate();
}

TerminalMode TUIRenderer::terminalMode() const noexcept {
    return activeTerminalMode_;
}

void TUIRenderer::setFullRedrawThreshold(float threshold) {
    fullRedrawThreshold_ = std::clamp(threshold, 0.0f, 1.0f);
}

void TUIRenderer::invalidate() {
    forceFullRedraw_ = true;
}

void TUIRenderer::activate() {
    graphicManager_.TUIEnterAltScreen();
    graphicManager_.TUIHideCursor();
    graphicManager_.TUIClear();
    graphicManager_.TUICursorHome();

    invalidate();
}

void TUIRenderer::restore() {
    graphicManager_.TUIResetColors();
    graphicManager_.TUIShowCursor();
    graphicManager_.TUILeaveAltScreen();
}

} // namespace TUI

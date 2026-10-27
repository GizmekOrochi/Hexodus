#ifndef TUIRENDERER_HPP
#define TUIRENDERER_HPP

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "TUIGraphicManager.hpp"
#include "TUIRendering.hpp"
#include "TUI-Rendering/Scene.hpp"

namespace TUI {

enum class TerminalMode {
    Automatic,
    UnicodeQuad,
    LinuxConsole
};

enum class PixelDrawingMode {
    Normal,
    Contrasted
};

class TUIRenderer {
private:
    friend struct RendererTestAccess;
    struct EncodedCell {
        std::string_view glyph{};
        RGBPanel foreground{};
        RGBPanel background{};

        [[nodiscard]] bool operator==(
            const EncodedCell& other
        ) const noexcept;

        [[nodiscard]] bool operator!=(
            const EncodedCell& other
        ) const noexcept {
            return !(*this == other);
        }
    };

    struct PixelCommand {
        int x;
        int y;
        Pixel color;
        PixelDrawingMode mode;
    };

    std::vector<Pixel> framebuffer_;
    std::vector<PixelCommand> pixelCommands_;
    std::vector<EncodedCell> currentCells_;
    std::vector<EncodedCell> previousCells_;

    TUIGraphicManager graphicManager_;

    std::vector<Scene*> scenes_;
    Scene* activeScene_;

    TerminalMode requestedTerminalMode_;
    TerminalMode activeTerminalMode_;

    int previousTerminalWidth_{};
    int previousTerminalHeight_{};
    bool forceFullRedraw_{true};

    // Above this ratio, rewriting the complete frame is generally cheaper
    // than emitting many cursor movements and short runs.
    float fullRedrawThreshold_{0.45f};

    void renderScene(std::size_t index);
    void renderScenes();
    void applyPixelCommands();

    void encodeFramebuffer(
        int terminalWidth,
        int terminalHeight
    );

    void encodeUnicodeFramebuffer(
        int terminalWidth,
        int terminalHeight
    );

    void encodeLinuxConsoleFramebuffer(
        int terminalWidth,
        int terminalHeight
    );

    void drawFullFrame(
        int terminalWidth,
        int terminalHeight
    );

    void drawDifferentialFrame(
        int terminalWidth,
        int terminalHeight
    );

    void appendCell(
        const EncodedCell& cell,
        RGBPanel& lastForeground,
        RGBPanel& lastBackground,
        bool& hasLastColors
    );

    [[nodiscard]] int horizontalPixelsPerCell() const noexcept;
    [[nodiscard]] int verticalPixelsPerCell() const noexcept;

    [[nodiscard]] static TerminalMode detectTerminalMode() noexcept;

    [[nodiscard]] static EncodedCell encodeQuad(
        const Pixel& topLeft,
        const Pixel& topRight,
        const Pixel& bottomLeft,
        const Pixel& bottomRight
    );

    [[nodiscard]] static std::string_view glyphForMask(
        std::uint8_t mask
    );

    [[nodiscard]] static int colorDistanceSquared(
        const Pixel& lhs,
        const Pixel& rhs
    );

    [[nodiscard]] static RGBPanel toRGBPanel(
        const Pixel& pixel
    );

    [[nodiscard]] static bool sameColor(
        const RGBPanel& lhs,
        const RGBPanel& rhs
    ) noexcept;

public:
    TUIRenderer();

    void setActiveScene(Scene* scene);
    void InitScene(Scene* scene);

    void drawPixel(int x, int y, Pixel color);
    void drawContrastedPixel(int x, int y);
    void drawBuffer();

    [[nodiscard]] int logicalWidth();
    [[nodiscard]] int logicalHeight();

    void setTerminalMode(TerminalMode mode);
    [[nodiscard]] TerminalMode terminalMode() const noexcept;

    void setFullRedrawThreshold(float threshold);
    void invalidate();

    void activate();
    void restore();
};

} // namespace TUI

#endif // TUIRENDERER_HPP

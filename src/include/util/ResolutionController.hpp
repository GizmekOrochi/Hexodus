#ifndef TUI_RESOLUTION_CONTROLLER_HPP
#define TUI_RESOLUTION_CONTROLLER_HPP

#pragma once

#include "Terminal.hpp"

namespace TUI {

// Automatic : utilise toute la surface disponible.
// Capped    : utilise toute la surface jusqu'à une limite maximum.
// Fixed     : demande exactement une résolution logique donnée.
enum class ResolutionMode {
    Automatic,
    Capped,
    Fixed
};

struct RenderResolution {
    int width{0};
    int height{0};

    [[nodiscard]] bool valid() const noexcept {
        return width > 0 && height > 0;
    }

    [[nodiscard]] bool operator==(const RenderResolution&) const noexcept = default;
};

class ResolutionController {
private:
    ResolutionMode mode_{ResolutionMode::Automatic};

    RenderResolution minimum_{1, 1};
    RenderResolution maximum_{0, 0};
    RenderResolution fixed_{0, 0};

public:
    void setMode(ResolutionMode mode) noexcept;
    [[nodiscard]] ResolutionMode mode() const noexcept;

    void setMinimum(RenderResolution resolution);
    void setMinimum(int width, int height);
    [[nodiscard]] const RenderResolution& minimum() const noexcept;

    // width/height <= 0 signifie "pas de limite" sur cet axe.
    void setMaximum(RenderResolution resolution);
    void setMaximum(int width, int height);
    void clearMaximum() noexcept;
    [[nodiscard]] const RenderResolution& maximum() const noexcept;

    void setFixed(RenderResolution resolution);
    void setFixed(int width, int height);
    [[nodiscard]] const RenderResolution& fixed() const noexcept;

    // cellPixelWidth / cellPixelHeight permettent de convertir la taille
    // du terminal en résolution logique.
    // Exemple UnicodeQuad : 2 x 2.
    // Exemple LinuxConsole : 1 x 1.
    [[nodiscard]] RenderResolution resolve(
        TerminalSize terminalSize,
        int cellPixelWidth,
        int cellPixelHeight
    ) const noexcept;

    [[nodiscard]] bool supported(
        TerminalSize terminalSize,
        int cellPixelWidth,
        int cellPixelHeight
    ) const noexcept;
};

} // namespace TUI

#endif // TUI_RESOLUTION_CONTROLLER_HPP

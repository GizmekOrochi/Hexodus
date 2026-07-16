#ifndef TUIRENDERER_HPP
#define TUIRENDERER_HPP

#pragma once

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

#include "TUIGraphicManager.hpp"
#include "TUIRendering.hpp"
#include "TUI-Rendering/Scene.hpp"

namespace TUI {

class TUIRenderer {
private:
    struct EncodedCell {
        std::string_view glyph;
        RGBPanel foreground;
        RGBPanel background;
    };

    std::vector<Pixel> framebuffer_;
    TUIGraphicManager graphicManager_;

    std::vector<Scene*> scenes_;
    Scene* activeScene_;

    void renderScene(std::size_t index);
    void renderScenes();

    [[nodiscard]] EncodedCell encodeQuad(const Pixel& topLeft, const Pixel& topRight, const Pixel& bottomLeft, const Pixel& bottomRight) const;
    [[nodiscard]] static std::string_view glyphForMask(std::uint8_t mask);
    [[nodiscard]] static int colorDistanceSquared(const Pixel& lhs, const Pixel& rhs);
    [[nodiscard]] static RGBPanel toRGBPanel(const Pixel& pixel);

public:
    TUIRenderer();

    void setActiveScene(Scene* scene);
    void InitScene(Scene* scene);

    void drawBuffer();

    void activate();
    void restore();
};

} // namespace TUI

#endif // TUIRENDERER_HPP

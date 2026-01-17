#ifndef TUIRENDERER_HPP
#define TUIRENDERER_HPP

#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include "TUIGraphicManager.hpp"
#include "TUIRendering.hpp"

namespace TUI {

struct Pixel {
    uint8_t r, g, b, a;
};

class TUIRenderer {
private:
    std::vector<Pixel> framebuffer;
    TUIGraphicManager graphicManager;

public:
    TUIRenderer() : graphicManager() {};

    void setBackgroundRed();
    
    void drawBuffer();

    void restore();
};

} //namespace

#endif // TUIRENDERER_HPP

#ifndef TUIRENDERER_HPP
#define TUIRENDERER_HPP

#pragma once
#include <vector>
#include <iostream>
#include "TUIGraphicManager.hpp"
#include "TUIRendering.hpp"

namespace TUI {
    
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

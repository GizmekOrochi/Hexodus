#ifndef TUIRENDERER_HPP
#define TUIRENDERER_HPP

#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include "TUIGraphicManager.hpp"

namespace TUI {

struct Pixel {
    uint8_t r, g, b, a;
};

struct Verticals {
    uint16_t X_, Y_;
};


class TUIRenderer {
private:
    std::vector<Pixel> framebuffer;
    TUIGraphicManager graphicManager;

public:
    TUIRenderer() : graphicManager() {};

    void setBackgroundRed() {
        graphicManager.update();
        for(size_t y{}; y < graphicManager.getHeigth(); y++){
            std::vector<std::vector<Pixel>> newTab;
            for(size_t x{}; x < graphicManager.getWidth(); x++){
                framebuffer.push_back(Pixel{255, 0, 0, 255});
            }
        }
    };
    
    void drawBuffer() {
        graphicManager.TUIClear();
        graphicManager.update();
        graphicManager.TUICursorHome();
        //std::cout << "x : " << graphicManager.getHeigth() << ", y : " << graphicManager.getWidth();
        for(size_t y{}; y < graphicManager.getHeigth(); y++){
            graphicManager.TUICursorHome();
            graphicManager.TUImoveCursor(1 , y + 1);
            for(size_t x{}; x < graphicManager.getWidth(); x++){
                if(x == 0) graphicManager.TUIDisplayChar('D');
                graphicManager.TUIDisplayChar('H');
            }
        }
    }
};

} //namespace

#endif // TUIRENDERER_HPP

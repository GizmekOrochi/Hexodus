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
        for(int y{}; y < graphicManager.getHeigth(); y++){
            std::vector<std::vector<Pixel>> newTab;
            for(int x{}; x < graphicManager.getWidth(); x++){
                framebuffer.push_back(Pixel{255, 0, 0, 255});
            }
        }
    };
    
    void drawBuffer() {
        graphicManager.TUIEnterAltScreen();
        graphicManager.disableRawMode();
        graphicManager.TUIHideCursor();
        graphicManager.TUIClear();
        graphicManager.TUICursorHome();
        graphicManager.TUISetColor(TUI::RGBPanel{0, 0, 160}, TUI::RGBPanel{0, 241, 24});

        int h = graphicManager.getHeigth();
        int w = graphicManager.getWidth();
        
        for (int y = 0; y < h; y++) {
            graphicManager.TUImoveCursor(y + 1, 1);
            for (int x = 0; x < w - 1; x++) {
                graphicManager.TUIDisplayChar("█");
            }
        }
    }


    void restore() {
        graphicManager.TUIResetColors();
        graphicManager.enableRawMode();
        graphicManager.TUIShowCursor();
        graphicManager.TUILeaveAltScreen();
    }
};

} //namespace

#endif // TUIRENDERER_HPP

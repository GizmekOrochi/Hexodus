#include "../include/TUIRenderer.hpp"

namespace TUI {

void TUIRenderer::setBackgroundRed() {
    graphicManager.update();
    for(int y{}; y < graphicManager.getHeigth(); y++){
        std::vector<std::vector<Pixel>> newTab;
        for(int x{}; x < graphicManager.getWidth(); x++){
            framebuffer.push_back(Pixel{255, 0, 0, 255});
        }
    }
};

void TUIRenderer::drawBuffer() {
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
            float alphaChannel{framebuffer[10 * y + x].a / 255.f};
            float RedChannel{framebuffer[10 * y + x].r / alphaChannel};
            float GreenChannel{framebuffer[10 * y + x].g / alphaChannel};
            float BlueChannel{framebuffer[10 * y + x].b / alphaChannel};
            
            TUI::RGBPanel ColorToDiplay{static_cast<uint8_t>(RedChannel), static_cast<uint8_t>(GreenChannel), static_cast<uint8_t>(BlueChannel)};
            
            graphicManager.TUISetColor(ColorToDiplay, ColorToDiplay);
            graphicManager.TUIDisplayChar(TUI::QuadBlock::empty);
        }
    }
}


void TUIRenderer::restore() {
    graphicManager.TUIResetColors();
    graphicManager.enableRawMode();
    graphicManager.TUIShowCursor();
    graphicManager.TUILeaveAltScreen();
}

} //namespace
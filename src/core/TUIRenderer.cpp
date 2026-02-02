#include "../include/TUIRenderer.hpp"

namespace TUI {

#include <cassert>

void TUIRenderer::RenderScenes() {
    assert(activeScene_ != nullptr && "TUIRenderer::RenderScenes(): activeScene_ is null");
    graphicManager.update();
    framebuffer.clear();
    framebuffer.reserve(graphicManager.getWidth() * graphicManager.getHeigth());
    for(int y{}; y < graphicManager.getHeigth(); y++){
        for(int x{}; x < graphicManager.getWidth(); x++){
            framebuffer.push_back(Pixel{0, 0, 0, 255});
        }
    }
    size_t activeSceneIndex{INT_MAX};
    for(size_t i{}; i < Scenes_.size(); i++) {
        if(Scenes_[i] == activeScene_) {
            activeSceneIndex = i;
            continue;
        }
        RenderScene(i);
    }
    if(activeSceneIndex != INT_MAX) RenderScene(activeSceneIndex);
}


void TUIRenderer::RenderScene(size_t index) {
    int h = graphicManager.getHeigth();
    int w = graphicManager.getWidth();

    int SceneOrigin_X = (Scenes_[index]->Origin()[0] * w) / 100;
    int SceneOrigin_Y = (Scenes_[index]->Origin()[1] * h) / 100;
    
    int SceneEnding_X = (Scenes_[index]->Ending()[0] * w) / 100;
    int SceneEnding_Y = (Scenes_[index]->Ending()[1] * h) / 100;

    int maxXBuffer = SceneEnding_X - SceneOrigin_X;
    int maxYBuffer = SceneEnding_Y - SceneOrigin_Y;

    std::vector<Pixel> SceneBuffer{Scenes_[index]->convertScene(h, w)};

    for (int y = 0; y < maxYBuffer; y++) {
        for (int x = 0; x < maxXBuffer; x++) {
            int screenX = SceneOrigin_X + x;
            int screenY = SceneOrigin_Y + y;
            if (screenX < 0 || screenX >= w) continue;
            if (screenY < 0 || screenY >= h) continue;
            Pixel src = SceneBuffer[y * maxXBuffer + x];
            // if alphaChannel == 0 then not need to desplay
            if (src.a == 0) continue;
            framebuffer[screenY * w + screenX] = src;
        }
    }
};


void TUIRenderer::InitScene(Scene* scene) {
    Scenes_.push_back(scene);
    activeScene_ = scene;
}


void TUIRenderer::drawBuffer() {
    graphicManager.TUICursorHome();
    
    RenderScenes();

    int h = graphicManager.getHeigth();
    int w = graphicManager.getWidth();

    TUI::RGBPanel lastColor{255, 255, 255};

    for (int y = 0; y < h; y++) {
        //graphicManager.TUImoveCursor(y + 1, 1); will be usfull when the feature of displaying only the active scene will be activated
        for (int x = 0; x < w; x++) {
            float alphaChannel{framebuffer[y * w + x].a / 255.f};
            float RedChannel{framebuffer[y * w + x].r / alphaChannel};
            float GreenChannel{framebuffer[y * w + x].g / alphaChannel};
            float BlueChannel{framebuffer[y * w + x].b / alphaChannel};
            
            TUI::RGBPanel ColorToDiplay{static_cast<uint8_t>(RedChannel), static_cast<uint8_t>(GreenChannel), static_cast<uint8_t>(BlueChannel)};
            
            if(ColorToDiplay.R_ == lastColor.R_ && ColorToDiplay.G_ == lastColor.G_ && ColorToDiplay.B_ == lastColor.B_) graphicManager.addPixel(TUI::QuadBlock::empty);
            else graphicManager.addPixel(TUI::QuadBlock::empty, ColorToDiplay, ColorToDiplay);
        }
    }
    graphicManager.execCommand();
}


void TUIRenderer::setActiveScene(Scene* Scene) { this->activeScene_ = Scene; };

void TUIRenderer::activate() {
    graphicManager.TUIEnterAltScreen();
    //graphicManager.enableRawMode();   Disable the charinput in commant for now until I implement the polling
    graphicManager.TUIHideCursor();
    graphicManager.TUIClear();
    graphicManager.TUICursorHome();
}

void TUIRenderer::restore() {
    graphicManager.TUIResetColors();
    graphicManager.disableRawMode();
    graphicManager.TUIShowCursor();
    graphicManager.TUILeaveAltScreen();
}

} //namespace
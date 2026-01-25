#ifndef TUIRENDERER_HPP
#define TUIRENDERER_HPP

#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <cstdint>
#include "TUIGraphicManager.hpp"
#include "TUIRendering.hpp"
#include "TUI-Rendering/Scene2D.hpp"
#include "TUI-Rendering/Scene3D.hpp"

namespace TUI {
    
class TUIRenderer {
private:
    std::vector<Pixel> framebuffer;
    TUIGraphicManager graphicManager;
    // lATER MULTIPLE RENDERING
    //std::vector<std::unique_ptr<Scene>> Scenes_;
    //Scene* activeScene_ = nullptr; // scene that the user can interact with
    Scene2D* scene_;

public:
    TUIRenderer() : framebuffer{}, graphicManager{}, scene_{nullptr} {};

    //void setActiveScene(Scene& Scene); LATER

    void setBackgroundRed();

    //origin is the origin point ( in 100% of the screen) same for ending
    void InitScene(Scene2D* scene);
    
    void drawBuffer();

    void restore();
};

} //namespace

#endif // TUIRENDERER_HPP

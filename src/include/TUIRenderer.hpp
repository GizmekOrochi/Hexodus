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
    std::vector<Scene*> Scenes_;
    Scene* activeScene_ = nullptr; // scene that the user can interact with

    void RenderScene(size_t index);
    void RenderScenes();
public:
    TUIRenderer() : framebuffer{}, graphicManager{}, Scenes_{} {};

    void setActiveScene(Scene* Scene);

    //origin is the origin point ( in 100% of the screen) same for ending
    void InitScene(Scene* scene);
    
    void drawBuffer();

    void restore();
};

} //namespace

#endif // TUIRENDERER_HPP

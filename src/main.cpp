#include <string>
#include "include/TUIRenderer.hpp"
#include <iostream>

int main() {

    TUI::TUIRenderer renderer{};
    Scene3D* scene = new Scene3D{{0, 0}, {100, 100}};
    scene->setBackground(TUI::Pixel{255, 0, 0, 0});
    scene->getCamera().setPosition({0.f, 0.f, 0.f, 1.f});
    scene->addTriangle(Triangle{
        {0.f, 0.f, 50.f, 1.f},
        {10.f, 0.f, 50.f, 1.f},
        {0.f, 10.f, 50.f, 1.f},
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255}
    });

    renderer.InitScene(scene);
    renderer.drawBuffer();
    
    while (true)
    {

    }

    return 0;
}
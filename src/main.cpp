#include <string>
#include "include/TUIRenderer.hpp"
#include <iostream>
#include <string>

int main() {

    TUI::TUIRenderer renderer{};

    Scene3D* scene  = new Scene3D({0, 0}, {50, 100});
    Scene2D* scene2 = new Scene2D({50, 0}, {100, 100});

    renderer.InitScene(scene);
    renderer.InitScene(scene2);

    scene->setBackground(TUI::Pixel{0, 0, 0, 255});
    scene2->setBackground(TUI::Pixel{0, 0, 255, 255});

    scene->getCamera().setPosition({0.f, 0.f, 0.f, 1.f});
    scene->getCamera().rotate(1.f, 1.f);

    size_t triangle{scene->addTriangle(Triangle{
        {-10.f, 0.f, 50.f, 1.f},
        {10.f, 0.f, 50.f, 1.f},
        {0.f, 10.f, 50.f, 1.f},
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{0, 255, 0, 255},
        TUI::Pixel{0, 0, 255, 255}
    })};

    scene2->add(
        std::make_unique<UI::UISquare>(
            std::array<int, 2>{100, 100},
            std::array<int, 2>{700, 700},
            1,
            TUI::Pixel{255, 0, 0, 255}
        )
    );

    renderer.activate();
    renderer.drawBuffer();
    while (true) {
        renderer.drawBuffer();
        //scene->Element(triangle).rotateX(0.1f);
        scene->Element(triangle).rotateZ(0.1f);
        //scene->Element(triangle).translate(0.1f, 0.f, 0.f);
        //scene->getCamera().moveForward(0.001f);
        //scene->getCamera().moveUp(0.001f);
        //scene->getCamera().rotate(0.01f, 0.f);
        usleep(16'000); // ~60 FPS
    }
    return 0;
}

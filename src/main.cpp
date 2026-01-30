#include <string>
#include "include/TUIRenderer.hpp"
#include <iostream>

int main() {

    TUI::TUIRenderer renderer{};

    Scene3D* scene  = new Scene3D({0, 0}, {50, 100});
    Scene2D* scene2 = new Scene2D({50, 0}, {100, 100});

    renderer.InitScene(scene);
    renderer.InitScene(scene2);

    scene->setBackground(TUI::Pixel{0, 255, 0, 255});
    scene2->setBackground(TUI::Pixel{0, 0, 255, 255});

    scene->getCamera().setPosition({0.f, 0.f, 0.f, 1.f});
    scene->addTriangle(Triangle{
        {0.f, 0.f, 50.f, 1.f},
        {10.f, 0.f, 50.f, 1.f},
        {0.f, 10.f, 50.f, 1.f},
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255}
    });

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
        usleep(16'000); // ~60 FPS
    }
    return 0;
}

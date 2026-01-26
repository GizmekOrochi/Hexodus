#include <string>
#include "include/TUIRenderer.hpp"
#include <iostream>

int main() {

    TUI::TUIRenderer renderer{};
    Scene2D* scene = new Scene2D{{0, 0}, {100, 50}};
    Scene2D* scene2 = new Scene2D{{0, 50}, {100, 100}};
    scene->setBackground(TUI::Pixel{255, 0, 0, 0});
    // adding red rectangle from 10.10% to 50,50 % if the scene
    scene->add(std::make_unique<UISquare>(
        std::array<int,2>{100, 100},
        std::array<int,2>{500,500},
        1,
        TUI::Pixel{255,0,0,255}
    ));
    // adding green rectangle filling all the scene
    scene->add(std::make_unique<UISquare>(
        std::array<int,2>{0, 0},
        std::array<int,2>{1000,1000},
        2,
        TUI::Pixel{0,255,0,255}
    ));
    // adding blue rectangle rectangle filling all the scene
    scene2->add(std::make_unique<UISquare>(
        std::array<int,2>{0, 0},
        std::array<int,2>{1000,1000},
        1,
        TUI::Pixel{0,0,255,255}
    ));
    renderer.InitScene(scene);
    renderer.InitScene(scene2);
    
    while (true)
    {
        renderer.drawBuffer();
    }

    return 0;
}
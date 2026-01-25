#include <string>
#include "include/TUIRenderer.hpp"
#include <iostream>

int main() {

    TUI::TUIRenderer renderer{};
    Scene2D* scene = new Scene2D({0, 0}, {100, 100});
    scene->setBackground(TUI::Pixel{255, 0, 0, 0});
    /*
    scene->add(std::make_unique<UISquare>( all black
        std::array<int,2>{100,100},
        std::array<int,2>{100,600},
        1,
        TUI::Pixel{255,0,0,255}
    ));
    */
    scene->add(std::make_unique<UISquare>( // all red
        std::array<int,2>{0, 0},
        std::array<int,2>{500,500},
        1,
        TUI::Pixel{255,0,0,255}
    ));
    scene->add(std::make_unique<UISquare>( // all red
        std::array<int,2>{0, 0},
        std::array<int,2>{700,500},
        2,
        TUI::Pixel{0,255,0,255}
    ));
    renderer.InitScene(scene);
    renderer.drawBuffer();
    while (true)
    {
        /* code */
    }

    return 0;
}
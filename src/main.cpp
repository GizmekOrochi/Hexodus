#include <string>
#include "include/TUIRenderer.hpp"
#include <iostream>
#include <string>

int main() {
    TUI::TUIRenderer renderer{};

    Scene3D* scene = new Scene3D({0, 0}, {50, 100});
    Scene2D* scene2 = new Scene2D({50, 0}, {100, 100});

    renderer.InitScene(scene);
    renderer.InitScene(scene2);

    scene->setBackground(TUI::Pixel{0, 0, 0, 255});
    scene2->setBackground(TUI::Pixel{0, 0, 255, 255});

    scene->getCamera().setPosition({0.f, 0.f, -15.f, 1.f});
    scene->getCamera().lookAt({0.f, 0.f, 0.f, 1.f});

    std::vector<Vector> cubeVertices = {
        {-5.f, -5.f, -5.f, 1.f},
        {5.f, -5.f, -5.f, 1.f},
        {5.f, 5.f, -5.f, 1.f},
        {-5.f, 5.f, -5.f, 1.f},
        {-5.f, -5.f, 5.f, 1.f},
        {5.f, -5.f, 5.f, 1.f},
        {5.f, 5.f, 5.f, 1.f},
        {-5.f, 5.f, 5.f, 1.f}
    };

    Vector cubeCenter = {0.f, 0.f, 0.f, 1.f};
    std::vector<size_t> triangleIndices;
    
    // Back face
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[0], cubeVertices[1], cubeVertices[2],
        TUI::Pixel{255, 0, 0, 255},   // Red
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255}
    }));
    
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[0], cubeVertices[2], cubeVertices[3],
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255}
    }));

    // Front face
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[4], cubeVertices[5], cubeVertices[6],
        TUI::Pixel{0, 255, 0, 255},   // Green
        TUI::Pixel{0, 255, 0, 255},
        TUI::Pixel{0, 255, 0, 255}
    }));
    
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[4], cubeVertices[6], cubeVertices[7],
        TUI::Pixel{0, 255, 0, 255},
        TUI::Pixel{0, 255, 0, 255},
        TUI::Pixel{0, 255, 0, 255}
    }));

    // Left face
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[0], cubeVertices[3], cubeVertices[7],
        TUI::Pixel{0, 0, 255, 255},   // Blue
        TUI::Pixel{0, 0, 255, 255},
        TUI::Pixel{0, 0, 255, 255}
    }));
    
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[0], cubeVertices[7], cubeVertices[4],
        TUI::Pixel{0, 0, 255, 255},
        TUI::Pixel{0, 0, 255, 255},
        TUI::Pixel{0, 0, 255, 255}
    }));

    // Right face
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[1], cubeVertices[2], cubeVertices[6],
        TUI::Pixel{255, 255, 0, 255},   // Yellow
        TUI::Pixel{255, 255, 0, 255},
        TUI::Pixel{255, 255, 0, 255}
    }));
    
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[1], cubeVertices[6], cubeVertices[5],
        TUI::Pixel{255, 255, 0, 255},
        TUI::Pixel{255, 255, 0, 255},
        TUI::Pixel{255, 255, 0, 255}
    }));

    // Top face
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[3], cubeVertices[2], cubeVertices[6],
        TUI::Pixel{255, 0, 255, 255},   // Magenta
        TUI::Pixel{255, 0, 255, 255},
        TUI::Pixel{255, 0, 255, 255}
    }));
    
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[3], cubeVertices[6], cubeVertices[7],
        TUI::Pixel{255, 0, 255, 255},
        TUI::Pixel{255, 0, 255, 255},
        TUI::Pixel{255, 0, 255, 255}
    }));

    // Bottom face
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[0], cubeVertices[1], cubeVertices[5],
        TUI::Pixel{0, 255, 255, 255},   // Cyan
        TUI::Pixel{0, 255, 255, 255},
        TUI::Pixel{0, 255, 255, 255}
    }));
    
    triangleIndices.push_back(scene->addTriangle(Triangle{
        cubeVertices[0], cubeVertices[5], cubeVertices[4],
        TUI::Pixel{0, 255, 255, 255},
        TUI::Pixel{0, 255, 255, 255},
        TUI::Pixel{0, 255, 255, 255}
    }));

    // Scene 2D exemple
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
        for (size_t i = 0; i < triangleIndices.size(); i++) {
            scene->Element(triangleIndices[i]).rotateY(0.1f, cubeCenter);
            scene->Element(triangleIndices[i]).rotateZ(0.1f, cubeCenter);
        }
        usleep(16'000); // ~60 FPS
    }
    
    return 0;
}
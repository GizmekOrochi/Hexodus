#include <array>
#include <cstddef>
#include <memory>
#include <vector>
#include <unistd.h>

#include "include/TUIRenderer.hpp"

int main() {
    TUI::TUIRenderer renderer{};

    Scene scene{{0, 0}, {100, 100}};
    renderer.InitScene(&scene);

    scene.setBackground(TUI::Pixel{0, 0, 0, 255});

    scene.getCamera().setPosition(
        Geometry::Vector{0.0f, 0.0f, -130.0f, 1.0f}
    );
    scene.getCamera().lookAt(
        Geometry::Vector{0.0f, 0.0f, 0.0f, 1.0f}
    );

    constexpr float cubeScale = 50.0f;

    const std::vector<Geometry::Vector> cubeVertices{
        {-cubeScale, -cubeScale, -cubeScale, 1.0f},
        { cubeScale, -cubeScale, -cubeScale, 1.0f},
        { cubeScale,  cubeScale, -cubeScale, 1.0f},
        {-cubeScale,  cubeScale, -cubeScale, 1.0f},
        {-cubeScale, -cubeScale,  cubeScale, 1.0f},
        { cubeScale, -cubeScale,  cubeScale, 1.0f},
        { cubeScale,  cubeScale,  cubeScale, 1.0f},
        {-cubeScale,  cubeScale,  cubeScale, 1.0f}
    };

    const Geometry::Vector cubeCenter{0.0f, 0.0f, 0.0f, 1.0f};
    std::vector<std::size_t> triangleIndices;

    const auto addTriangle = [&](const Geometry::Triangle& triangle) {
        triangleIndices.push_back(scene.addTriangle(triangle));
    };

    // Back face
    addTriangle(Geometry::Triangle{
        cubeVertices[0], cubeVertices[1], cubeVertices[2],
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{70, 0, 0, 50}
    });
    addTriangle(Geometry::Triangle{
        cubeVertices[0], cubeVertices[2], cubeVertices[3],
        TUI::Pixel{255, 0, 0, 255},
        TUI::Pixel{125, 0, 0, 50},
        TUI::Pixel{70, 0, 0, 50}
    });

    // Front face
    addTriangle(Geometry::Triangle{
        cubeVertices[5], cubeVertices[4], cubeVertices[6],
        TUI::Pixel{0, 255, 0, 255},
        TUI::Pixel{0, 125, 0, 255},
        TUI::Pixel{0, 70, 0, 255}
    });
    addTriangle(Geometry::Triangle{
        cubeVertices[6], cubeVertices[4], cubeVertices[7],
        TUI::Pixel{0, 255, 0, 255},
        TUI::Pixel{0, 125, 0, 255},
        TUI::Pixel{0, 70, 0, 255}
    });

    // Left face
    addTriangle(Geometry::Triangle{
        cubeVertices[0], cubeVertices[3], cubeVertices[7],
        TUI::Pixel{0, 0, 255, 255},
        TUI::Pixel{0, 0, 125, 255},
        TUI::Pixel{0, 0, 70, 255}
    });
    addTriangle(Geometry::Triangle{
        cubeVertices[0], cubeVertices[7], cubeVertices[4],
        TUI::Pixel{0, 0, 255, 255},
        TUI::Pixel{0, 0, 125, 255},
        TUI::Pixel{0, 0, 70, 255}
    });

    // Right face
    addTriangle(Geometry::Triangle{
        cubeVertices[2], cubeVertices[1], cubeVertices[6],
        TUI::Pixel{255, 255, 0, 255},
        TUI::Pixel{125, 125, 0, 255},
        TUI::Pixel{70, 70, 0, 255}
    });
    addTriangle(Geometry::Triangle{
        cubeVertices[6], cubeVertices[1], cubeVertices[5],
        TUI::Pixel{255, 255, 0, 255},
        TUI::Pixel{125, 125, 0, 255},
        TUI::Pixel{70, 70, 0, 255}
    });

    // Top face
    addTriangle(Geometry::Triangle{
        cubeVertices[3], cubeVertices[2], cubeVertices[6],
        TUI::Pixel{255, 0, 255, 255},
        TUI::Pixel{125, 0, 125, 255},
        TUI::Pixel{70, 0, 70, 255}
    });
    addTriangle(Geometry::Triangle{
        cubeVertices[3], cubeVertices[6], cubeVertices[7],
        TUI::Pixel{255, 0, 255, 255},
        TUI::Pixel{125, 0, 125, 255},
        TUI::Pixel{70, 0, 70, 255}
    });

    // Bottom face
    addTriangle(Geometry::Triangle{
        cubeVertices[1], cubeVertices[0], cubeVertices[5],
        TUI::Pixel{0, 255, 255, 255},
        TUI::Pixel{0, 125, 125, 255},
        TUI::Pixel{0, 70, 70, 255}
    });
    addTriangle(Geometry::Triangle{
        cubeVertices[5], cubeVertices[0], cubeVertices[4],
        TUI::Pixel{0, 255, 255, 255},
        TUI::Pixel{0, 125, 125, 255},
        TUI::Pixel{0, 70, 70, 255}
    });

    // The same Scene also contains UI elements.
    scene.addUIElement(
        std::make_unique<UI::UISquare>(
            std::array<int, 2>{700, 80},
            std::array<int, 2>{950, 300},
            1,
            TUI::Pixel{255, 0, 0, 180}
        )
    );

    renderer.activate();

    while (true) {
        renderer.drawBuffer();

        for (const std::size_t triangleIndex : triangleIndices) {
            scene.triangle(triangleIndex).rotateY(0.05f, cubeCenter);
            scene.triangle(triangleIndex).rotateZ(0.05f, cubeCenter);
        }

        usleep(16'000);
    }

    return 0;
}

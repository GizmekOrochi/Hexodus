#include <array>
#include <chrono>
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

#include "include/TUIInput.hpp"
#include "include/TUIRenderer.hpp"

int main() {
    TUI::TUIRenderer renderer{};
    TUI::TUIInput input{};

    bool rendererActive{false};
    bool inputActive{false};

    try {
        Scene scene{{0, 0}, {100, 100}};
        renderer.InitScene(&scene);

        scene.setBackground(TUI::Pixel{0, 0, 0, 255});

        scene.getCamera().setPosition(
            Geometry::Vector{0.0f, 0.0f, -130.0f, 1.0f}
        );

        scene.getCamera().lookAt(
            Geometry::Vector{0.0f, 0.0f, 0.0f, 1.0f}
        );

        constexpr float cubeScale{50.0f};

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

        const Geometry::Vector cubeCenter{
            0.0f, 0.0f, 0.0f, 1.0f
        };

        std::vector<std::size_t> triangleIndices;

        const auto addTriangle =
            [&](const Geometry::Triangle& triangle) {
                triangleIndices.push_back(
                    scene.addTriangle(triangle)
                );
            };

        addTriangle({
            cubeVertices[0], cubeVertices[1], cubeVertices[2],
            {255, 0, 0, 255},
            {255, 0, 0, 255},
            {70, 0, 0, 50}
        });

        addTriangle({
            cubeVertices[0], cubeVertices[2], cubeVertices[3],
            {255, 0, 0, 255},
            {125, 0, 0, 50},
            {70, 0, 0, 50}
        });

        addTriangle({
            cubeVertices[5], cubeVertices[4], cubeVertices[6],
            {0, 255, 0, 255},
            {0, 125, 0, 255},
            {0, 70, 0, 255}
        });

        addTriangle({
            cubeVertices[6], cubeVertices[4], cubeVertices[7],
            {0, 255, 0, 255},
            {0, 125, 0, 255},
            {0, 70, 0, 255}
        });

        addTriangle({
            cubeVertices[0], cubeVertices[3], cubeVertices[7],
            {0, 0, 255, 255},
            {0, 0, 125, 255},
            {0, 0, 70, 255}
        });

        addTriangle({
            cubeVertices[0], cubeVertices[7], cubeVertices[4],
            {0, 0, 255, 255},
            {0, 0, 125, 255},
            {0, 0, 70, 255}
        });

        addTriangle({
            cubeVertices[2], cubeVertices[1], cubeVertices[6],
            {255, 255, 0, 255},
            {125, 125, 0, 255},
            {70, 70, 0, 255}
        });

        addTriangle({
            cubeVertices[6], cubeVertices[1], cubeVertices[5],
            {255, 255, 0, 255},
            {125, 125, 0, 255},
            {70, 70, 0, 255}
        });

        addTriangle({
            cubeVertices[3], cubeVertices[2], cubeVertices[6],
            {255, 0, 255, 255},
            {125, 0, 125, 255},
            {70, 0, 70, 255}
        });

        addTriangle({
            cubeVertices[3], cubeVertices[6], cubeVertices[7],
            {255, 0, 255, 255},
            {125, 0, 125, 255},
            {70, 0, 70, 255}
        });

        addTriangle({
            cubeVertices[1], cubeVertices[0], cubeVertices[5],
            {0, 255, 255, 255},
            {0, 125, 125, 255},
            {0, 70, 70, 255}
        });

        addTriangle({
            cubeVertices[5], cubeVertices[0], cubeVertices[4],
            {0, 255, 255, 255},
            {0, 125, 125, 255},
            {0, 70, 70, 255}
        });

        scene.addUIElement(
            std::make_unique<UI::UISquare>(
                std::array<int, 2>{700, 80},
                std::array<int, 2>{950, 300},
                1,
                TUI::Pixel{255, 0, 0, 180}
            )
        );

        renderer.activate();
        rendererActive = true;

        input.activate();
        inputActive = true;

        using Clock = std::chrono::steady_clock;
        auto previousTime{Clock::now()};

        bool running{true};
        bool rotateCube{true};

        while (running) {
            const auto currentTime{Clock::now()};

            float deltaTime{
                std::chrono::duration<float>(
                    currentTime - previousTime
                ).count()
            };

            previousTime = currentTime;

            if (deltaTime > 0.1f) {
                deltaTime = 0.1f;
            }

            input.pollEvents();

            if (
                input.quitRequested() ||
                input.keyPressed(TUI::Key::Escape)
            ) {
                running = false;
                continue;
            }

            Rendering::Camera& camera{scene.getCamera()};

            if (input.keyDown(TUI::Key::Z)) {
                camera.moveForward(deltaTime);
            }

            if (input.keyDown(TUI::Key::S)) {
                camera.moveForward(-deltaTime);
            }

            if (input.keyDown(TUI::Key::Q)) {
                camera.moveRight(-deltaTime);
            }

            if (input.keyDown(TUI::Key::D)) {
                camera.moveRight(deltaTime);
            }

            if (input.keyDown(TUI::Key::Space)) {
                camera.moveUp(deltaTime);
            }

            if (input.keyDown(TUI::Key::C)) {
                camera.moveUp(-deltaTime);
            }

            constexpr float rotationSpeed{2.5f};

            if (input.keyDown(TUI::Key::ArrowLeft)) {
                camera.rotate(-rotationSpeed * deltaTime, 0.0f);
            }

            if (input.keyDown(TUI::Key::ArrowRight)) {
                camera.rotate(rotationSpeed * deltaTime, 0.0f);
            }

            if (input.keyDown(TUI::Key::ArrowUp)) {
                camera.rotate(0.0f, -rotationSpeed * deltaTime);
            }

            if (input.keyDown(TUI::Key::ArrowDown)) {
                camera.rotate(0.0f, rotationSpeed * deltaTime);
            }

            if (input.keyPressed(TUI::Key::R)) {
                rotateCube = !rotateCube;
            }

            if (rotateCube) {
                const float rotation{1.5f * deltaTime};

                for (const std::size_t index : triangleIndices) {
                    Geometry::Triangle& triangle{
                        scene.triangle(index)
                    };

                    triangle = triangle.rotateY(
                        rotation,
                        cubeCenter
                    );

                    triangle = triangle.rotateZ(
                        rotation,
                        cubeCenter
                    );
                }
            }

            renderer.drawBuffer();
        }

        input.restore();
        inputActive = false;

        renderer.restore();
        rendererActive = false;

        return 0;
    }
    catch (const std::exception& exception) {
        if (inputActive) {
            input.restore();
        }

        if (rendererActive) {
            renderer.restore();
        }

        std::cerr
            << "Hexodus error: "
            << exception.what()
            << '\n';

        return 1;
    }
}
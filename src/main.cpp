#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <exception>
#include <iostream>

#include "include/TUIInput.hpp"
#include "include/TUIRenderer.hpp"

namespace {

using Geometry::Triangle;
using Geometry::Vector;
using TUI::Pixel;

// Ajoute un quadrilatère sous forme de deux triangles.
void addQuad(Scene& scene, const Vector& a, const Vector& b, const Vector& c, const Vector& d, const Pixel& colorA, const Pixel& colorB, const Pixel& colorC, const Pixel& colorD) {
    scene.addTriangle(Triangle{a, b, c, colorA, colorB, colorC});
    scene.addTriangle(Triangle{a, c, d, colorA, colorC, colorD});
}

// Ajoute une boîte rectangulaire fermée.
void addBox(Scene& scene, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, Pixel mainColor) {
    const Vector p000{minX, minY, minZ, 1.0f};
    const Vector p100{maxX, minY, minZ, 1.0f};
    const Vector p110{maxX, maxY, minZ, 1.0f};
    const Vector p010{minX, maxY, minZ, 1.0f};

    const Vector p001{minX, minY, maxZ, 1.0f};
    const Vector p101{maxX, minY, maxZ, 1.0f};
    const Vector p111{maxX, maxY, maxZ, 1.0f};
    const Vector p011{minX, maxY, maxZ, 1.0f};

    const auto darken = [](Pixel color, float factor) {
        return Pixel{
            static_cast<std::uint8_t>(static_cast<float>(color.r) * factor), 
            static_cast<std::uint8_t>(static_cast<float>(color.g) * factor), 
            static_cast<std::uint8_t>(static_cast<float>(color.b) * factor), color.a
        };
    };

    const Pixel frontColor{mainColor};
    const Pixel backColor{darken(mainColor, 0.55f)};
    const Pixel leftColor{darken(mainColor, 0.70f)};
    const Pixel rightColor{darken(mainColor, 0.85f)};
    const Pixel topColor{darken(mainColor, 1.0f)};
    const Pixel bottomColor{darken(mainColor, 0.35f)};

    /*
     * Tous les sommets utilisent le même winding.
     * Les normales sont dirigées vers l’extérieur de la boîte.
     */

    // Face avant : normale -Z
    addQuad(scene, p000, p010, p110, p100, frontColor, frontColor, frontColor, frontColor);

    // Face arrière : normale +Z
    addQuad(scene, p001, p101, p111, p011, backColor, backColor, backColor, backColor);

    // Face gauche : normale -X
    addQuad(scene, p000, p001, p011, p010, leftColor, leftColor, leftColor, leftColor);

    // Face droite : normale +X
    addQuad(scene, p100, p110, p111, p101, rightColor, rightColor, rightColor, rightColor);

    // Face supérieure : normale +Y
    addQuad(scene, p010, p011, p111, p110, topColor, topColor, topColor, topColor);

    // Face inférieure : normale -Y
    addQuad(scene, p000, p100, p101, p001, bottomColor, bottomColor, bottomColor, bottomColor);
}

// Ajoute une dalle horizontale.
void addFloorTile(Scene& scene, float minX, float minZ, float maxX, float maxZ, float y, Pixel color) {
    const Vector p00{minX, y, minZ, 1.0f};
    const Vector p01{minX, y, maxZ, 1.0f};
    const Vector p11{maxX, y, maxZ, 1.0f};
    const Vector p10{maxX, y, minZ, 1.0f};

    // Normale dirigée vers +Y.
    addQuad(scene, p00, p01, p11, p10, color, color, color, color);
}

void buildTestMap(Scene& scene) {
    constexpr float mapHalfWidth{90.0f};
    constexpr float mapStartZ{-60.0f};
    constexpr float mapEndZ{260.0f};

    // Sol quadrillé
    constexpr float tileSize{20.0f};

    for (int zIndex{0}; zIndex < 16; ++zIndex) {
        for (int xIndex{0}; xIndex < 9; ++xIndex) {
            const float minX{-mapHalfWidth + static_cast<float>(xIndex) * tileSize};
            const float minZ{mapStartZ + static_cast<float>(zIndex) * tileSize};

            const bool alternate{((xIndex + zIndex) % 2) == 0};

            const Pixel tileColor{alternate ? Pixel{35, 40, 48, 255} : Pixel{22, 26, 32, 255}};

            addFloorTile(scene, minX, minZ, minX + tileSize, minZ + tileSize, 0.0f, tileColor);
        }
    }

    // Murs périphériques
    addBox(scene, -95.0f, 0.0f, mapStartZ, -85.0f, 35.0f, mapEndZ, Pixel{70, 90, 125, 255});
    addBox(scene, 85.0f, 0.0f, mapStartZ, 95.0f, 35.0f, mapEndZ, Pixel{70, 90, 125, 255});
    addBox(scene, -95.0f, 0.0f, mapEndZ - 10.0f, 95.0f, 35.0f, mapEndZ, Pixel{90, 75, 120, 255});

    // Entrée de la map
    addBox(scene, -95.0f, 0.0f, mapStartZ, -25.0f, 35.0f, mapStartZ + 10.0f, Pixel{90, 75, 120, 255});
    addBox(scene, 25.0f, 0.0f, mapStartZ, 95.0f, 35.0f, mapStartZ + 10.0f, Pixel{90, 75, 120, 255});

    // Piliers de l'entrée
    addBox(scene, -27.0f, 0.0f, mapStartZ, -20.0f, 45.0f, mapStartZ + 18.0f, Pixel{160, 80, 65, 255});
    addBox(scene, 20.0f, 0.0f, mapStartZ, 27.0f, 45.0f, mapStartZ + 18.0f, Pixel{160, 80, 65, 255});

    // Caisses près du début
    addBox(scene, -55.0f, 0.0f, -10.0f, -30.0f, 24.0f, 15.0f, Pixel{150, 95, 45, 255});
    addBox(scene, 32.0f, 0.0f, 10.0f, 55.0f, 18.0f, 32.0f, Pixel{145, 90, 40, 255});
    addBox(scene, 40.0f, 18.0f, 14.0f, 58.0f, 34.0f, 30.0f, Pixel{180, 115, 55, 255});

    // Couloir central
    addBox(scene, -65.0f, 0.0f, 55.0f, -25.0f, 25.0f, 65.0f, Pixel{70, 115, 95, 255});
    addBox(scene, 25.0f, 0.0f, 55.0f, 65.0f, 25.0f, 65.0f, Pixel{70, 115, 95, 255});

    // Piliers centraux
    addBox(scene, -10.0f, 0.0f, 85.0f, 10.0f, 45.0f, 105.0f, Pixel{115, 80, 145, 255});
    addBox(scene, -72.0f, 0.0f, 115.0f, -52.0f, 38.0f, 135.0f, Pixel{95, 125, 170, 255});
    addBox(scene, 52.0f, 0.0f, 115.0f, 72.0f, 38.0f, 135.0f, Pixel{95, 125, 170, 255});

    // Petites couvertures
    addBox(scene, -45.0f, 0.0f, 155.0f, -20.0f, 14.0f, 165.0f, Pixel{130, 60, 70, 255});
    addBox(scene, 20.0f, 0.0f, 175.0f, 45.0f, 14.0f, 185.0f, Pixel{130, 60, 70, 255});

    // Plateforme finale
    addBox(scene, -50.0f, 0.0f, 205.0f, 50.0f, 10.0f, 245.0f, Pixel{60, 100, 135, 255});

    // Escaliers simplifiés
    addBox(scene, -18.0f, 0.0f, 185.0f, 18.0f, 3.0f, 193.0f, Pixel{80, 120, 150, 255});
    addBox(scene, -18.0f, 0.0f, 193.0f, 18.0f, 6.0f, 201.0f, Pixel{80, 120, 150, 255});
    addBox(scene, -18.0f, 0.0f, 201.0f, 18.0f, 9.0f, 209.0f, Pixel{80, 120, 150, 255});

    // Objectif lumineux
    addBox(scene, -9.0f, 10.0f, 222.0f, 9.0f, 35.0f, 240.0f, Pixel{30, 220, 180, 255});
}

} // namespace

int main() {
    TUI::TUIRenderer renderer{};
    TUI::TUIInput input{};

    bool rendererActive{false};
    bool inputActive{false};

    try {
        Scene scene{{0, 0}, {100, 100}};
        renderer.InitScene(&scene);

        scene.setBackground(TUI::Pixel{4, 6, 12, 255});
        scene.setBackfaceCulling(true);

        buildTestMap(scene);

        Rendering::Camera& camera{scene.getCamera()};

        camera.setPosition(Vector{0.0f, 12.0f, -45.0f, 1.0f});

        camera.lookAt(Vector{0.0f, 10.0f, 30.0f, 1.0f});

        renderer.activate();
        rendererActive = true;

        input.activate();
        inputActive = true;

        input.setMouseSensitivity(0.25f);

        using Clock = std::chrono::steady_clock;

        auto previousTime{Clock::now()};
        bool running{true};

        while (running) {
            const auto currentTime{Clock::now()};

            float deltaTime{std::chrono::duration<float>(currentTime - previousTime).count()};

            previousTime = currentTime;

            deltaTime = std::clamp(deltaTime, 0.0f, 0.1f);

            input.pollEvents();

            if (input.quitRequested() ||input.pressed(TUI::Scancode::Escape)) {
                running = false;
                continue;
            }

            // evdev reports physical key positions, independently of the keyboard layout.
            // KEY_W and KEY_A are labelled Z and Q on a standard French AZERTY keyboard.
            constexpr float keyboardRotationMultiplier{1.0f};
            constexpr float mouseRotationMultiplier{0.35f};
            const float movementDelta{deltaTime};

            const float keyboardRotationDelta{
                std::max(deltaTime, 1.0f / 60.0f) *
                keyboardRotationMultiplier
            };

            const float mouseYaw{
                static_cast<float>(input.mouseDeltaX()) *
                mouseRotationMultiplier
            };

            const float mousePitch{
                static_cast<float>(input.mouseDeltaY()) *
                mouseRotationMultiplier
            };


            if (input.down(TUI::Scancode::KeyW)) {
                camera.moveForward(movementDelta);
            }

            if (input.down(TUI::Scancode::KeyS)) {
                camera.moveForward(-movementDelta);
            }

            if (input.down(TUI::Scancode::KeyA)) {
                camera.moveRight(-movementDelta);
            }

            if (input.down(TUI::Scancode::KeyD)) {
                camera.moveRight(movementDelta);
            }

            if (input.down(TUI::Scancode::Space)) {
                camera.moveUp(movementDelta);
            }

            if (input.down(TUI::Scancode::LeftShift) || input.down(TUI::Scancode::RightShift)) {
                camera.moveUp(-movementDelta);
            }

            if (input.down(TUI::Scancode::ArrowLeft)) {
                camera.rotate(-keyboardRotationDelta, 0.0f);
            }

            if (input.down(TUI::Scancode::ArrowRight)) {
                camera.rotate(keyboardRotationDelta, 0.0f);
            }

            if (input.down(TUI::Scancode::ArrowUp)) {
                camera.rotate(0.0f, -keyboardRotationDelta);
            }

            if (input.down(TUI::Scancode::ArrowDown)) {
                camera.rotate(0.0f, keyboardRotationDelta);
            }

            if (
                mouseYaw != 0.0f ||
                mousePitch != 0.0f
            ) {
                camera.rotate(
                    mouseYaw,
                    mousePitch
                );
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

        std::cerr << "Hexodus error: " << exception.what() << '\n';

        return 1;
    }
}
#ifndef SCENE_HPP
#define SCENE_HPP

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "../TUIRendering.hpp"
#include "Camera.hpp"
#include "Geometry/object/Triangle.hpp"
#include "UI/UI.hpp"

struct ClippingPoint {
    Geometry::Vector pos;
    TUI::Pixel color;
};

class Scene {
private:
    std::array<uint8_t, 2> origin_;
    std::array<uint8_t, 2> ending_;
    TUI::Pixel backgroundColor_;

    std::vector<Geometry::Triangle> triangles_;
    std::vector<std::unique_ptr<UI::UIElement>> uiElements_;

    Rendering::Camera camera_;
    bool backfaceCulling_;

    Geometry::Vector worldToCamera(const Geometry::Vector& position) const;
    bool projectPoint(const Geometry::Vector& input, Geometry::Vector& output, int framebufferWidth, int framebufferHeight) const;

    ClippingPoint intersectNearPlane(const ClippingPoint& first, const ClippingPoint& second) const;

    std::vector<Geometry::Triangle> clipTriangle( const Geometry::Triangle& triangle) const;
    std::vector<Geometry::Triangle> projectTriangles(int framebufferWidth, int framebufferHeight) const;

    void renderTriangles(std::vector<TUI::Pixel>& colorBuffer, std::vector<float>& depthBuffer, int width, int height) const;

    void renderUI(std::vector<TUI::Pixel>& colorBuffer, int width, int height) const;

public:
    Scene(std::array<uint8_t, 2> origin,std::array<uint8_t, 2> ending);

    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    void setBackground(TUI::Pixel color);

    TUI::Pixel Color() const;
    std::array<uint8_t, 2> Origin() const;
    std::array<uint8_t, 2> Ending() const;

    void Origin(const std::array<uint8_t, 2>& origin);
    void Ending(const std::array<uint8_t, 2>& ending);

    std::size_t addTriangle(const Geometry::Triangle& triangle);
    Geometry::Triangle& triangle(std::size_t index);
    const Geometry::Triangle& triangle(std::size_t index) const;
    void clearTriangles();

    std::size_t addUIElement(std::unique_ptr<UI::UIElement> element);
    std::size_t add(std::unique_ptr<UI::UIElement> element);

    UI::UIElement& uiElement(std::size_t index);
    const UI::UIElement& uiElement(std::size_t index) const;
    void clearUIElements();

    Rendering::Camera& getCamera();
    const Rendering::Camera& getCamera() const;

    void setBackfaceCulling(bool enabled);
    void toggleBackfaceCulling();
    bool backfaceCullingEnabled() const;

    std::vector<TUI::Pixel> convertScene(
        int outputHeight,
        int outputLength
    ) const;
};

#endif // SCENE_HPP

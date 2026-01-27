#ifndef Scene3D_HPP
#define Scene3D_HPP

#pragma once
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "Scene.hpp"
#include "Camera.hpp"
#include "Geometry/object/Triangle.hpp"
#include "../TUIRendering.hpp"

using namespace Rendering;

class Scene3D : public Scene {
private:
    std::vector<Geometry::Triangle> ObjectList;
    Camera camera_;

    // Transform a point from world position to camera related position
    Geometry::Vector worldToCamera(Geometry::Vector& point) const;

    // true if the pixel is in the scene false if not
    bool projectToScreen(const Geometry::Vector& point) const;

    // sort the triangle from the farthest to the nearest, it also remove the triangles that are hidden
    std::vector<Geometry::Triangle> applyPainterAlgorithm(std::vector<Geometry::Triangle>& input);

    void drawLine(std::vector<TUI::Pixel>& pixels, float x1, float y1, float x2, float y2, TUI::Pixel color, int height, int width) const;

public:
    Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending);

    void addTriangle(const Geometry::Triangle& triangle);
    void addTriangles(const std::vector<Geometry::Triangle>& triangles);
    
    void clearTriangles();

    Camera& getCamera();
    const Camera& getCamera() const;

    // int outputHeight, int outputLength are the actual size of the terminal transmitted by the TUI Renderer
    // applyPainterAlgorithm -> for each Triangle for each pixel in the scene if projectToScreen returns true then add to std::vector<TUI::Pixel>
    std::vector<TUI::Pixel> convertScene(int outputHeight, int outputLength);
};

#endif // Scene3D_HPP
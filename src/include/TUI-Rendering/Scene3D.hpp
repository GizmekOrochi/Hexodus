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
public:
    Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending);

    void addTriangle(const Triangle& triangle);
    void addTriangles(const std::vector<Triangle>& triangles);

    void clearTriangles();
    
    Camera& getCamera();
    const Camera& getCamera() const;

    // int outputHeight, int outputLength are the actual size of the terminal transmitted by the TUI Renderer
    // applyPainterAlgorithm -> for each Triangle for each pixel in the scene if projectToScreen returns true then add to std::vector<TUI::Pixel>
    std::vector<TUI::Pixel> convertScene(int outputHeight, int outputLength);
};

#endif // Scene3D_HPP
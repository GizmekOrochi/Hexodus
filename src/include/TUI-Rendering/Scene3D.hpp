#ifndef Scene3D_HPP
#define Scene3D_HPP

#pragma once
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include "Scene.hpp"
#include "Geometry/object/Triangle.hpp"
#include "../TUIRendering.hpp"

using namespace Geometry;

class Scene3D : public Scene {
private:
    std::vector<Triangle> ObjectList;
public:
    Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending);

    // int outputHeight, int outputLenght are the actual size of the terminal transmited by the TUI Renderer
    std::vector<TUI::Pixel> convertScene(int outputHeight, int outputLenght);
};


#endif // Scene3D_HPP

#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#pragma once
#include <string>
#include <vector>
#include "../Geometry/object/Triangle.hpp"
#include "../TUIRendering.hpp"

namespace Geometry {

class Rasterizer {
private:
    float worldX_Limit_;
    float worldY_Limit_;
    std::vector<Triangle> ObjectList;
public:
    Rasterizer(float worldX_Limit, float worldY_Limit);

    std::vector<TUI::Pixel> convertObject(int outputHeight, int outputLenght);
};

}

#endif // RASTERIZER_HPP

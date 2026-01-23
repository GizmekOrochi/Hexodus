#include "../../include/TUI-Rendering/Rasterizer.hpp"

namespace Geometry {

Rasterizer::Rasterizer(float worldX_Limit, float worldY_Limit) : worldX_Limit_{worldX_Limit}, worldY_Limit_{worldY_Limit}, ObjectList{} {};

std::vector<TUI::Pixel> Rasterizer::convertObject(int outputHeight, int outputLenght) {
    std::vector<TUI::Pixel> res{};
    for(Triangle triangle : ObjectList) {
        Vector AB = triangle.getB() - triangle.getA();
        Vector AC = triangle.getC() - triangle.getA();
        Vector n = AB.Cross(AC);
        // if n.dot( P - A ) = 0 then that is in the plan WESHHHHH
        // one problem left I can't tet all the points because the terminal have big pixels. I need to test 1 point per pixel
        // we need to make a projection of the termianl teorical screen ok the world to deternime with point are in with pixels
    
    }
    return res;
}

}//namespace
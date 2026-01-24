#include "../../include/TUI-Rendering/Scene3D.hpp"

Scene3D::Scene3D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending) 
    : Scene{origin, ending}, ObjectList{} {}

void applyPainterAlgorithm() {
    // sort the facesby their Z axis in this order lowes -> highest
    // remove the completly hiden faces
}

std::vector<TUI::Pixel> Scene3D::convertScene(int outputHeight, int outputLenght) {
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
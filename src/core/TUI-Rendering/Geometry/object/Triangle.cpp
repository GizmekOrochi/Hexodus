#include "../../../../include/TUI-Rendering/Geometry/object/Triangle.hpp"
#include "../../../../include/util/Logger.hpp"

namespace Geometry {

Triangle::Triangle(Vector A, Vector B, Vector C) 
    : Verticle_{A, B, C}, 
    color_{TUI::Pixel{255, 255, 255, 255}, TUI::Pixel{255, 255, 255, 255}, TUI::Pixel{255, 255, 255, 255}}, 
    gradiant_{100, 100, 100} {}

Triangle::Triangle(Vector A, Vector B, Vector C, TUI::Pixel colorA, TUI::Pixel colorB, TUI::Pixel colorC)
    : Verticle_{A, B, C}, 
    color_{colorA, colorB, colorC}, 
    gradiant_{100, 100, 100} {}

Triangle::Triangle(Vector A, Vector B, Vector C, TUI::Pixel colorA, TUI::Pixel colorB, TUI::Pixel colorC, uint8_t gradiantA, uint8_t gradiantB, uint8_t gradiantC)
    : Verticle_{A, B, C}, 
    color_{colorA, colorB, colorC}, 
    gradiant_{gradiantA, gradiantB, gradiantC} {}

Triangle::Triangle(const Triangle& other)
    : Verticle_{other.getA(), other.getB(), other.getC()}, 
    color_{other.getColorA(), other.getColorB(), other.getColorC()}, 
    gradiant_{other.getGradiantA(), other.getGradiantB(), other.getGradiantC()} {}


Vector Triangle::getA() const { return Verticle_[0]; }
Vector Triangle::getB() const { return Verticle_[1]; }
Vector Triangle::getC() const { return Verticle_[2]; }

TUI::Pixel Triangle::getColorA() const { return color_[0]; }
TUI::Pixel Triangle::getColorB() const { return color_[1]; }
TUI::Pixel Triangle::getColorC() const { return color_[2]; }

uint8_t Triangle::getGradiantA() const { return gradiant_[0]; }
uint8_t Triangle::getGradiantB() const { return gradiant_[1]; }
uint8_t Triangle::getGradiantC() const { return gradiant_[2]; }

Triangle Triangle::translate(float x, float y, float z) {
    Matrix translation = Matrix::createTranslation(x, y, z);
    return transform(translation);
}

Triangle Triangle::rotateX(float angle) {
    Matrix rotation = Matrix::createRotationX(angle);
    return transform(rotation);
}

Triangle Triangle::rotateY(float angle) {
    Matrix rotation = Matrix::createRotationY(angle);
    return transform(rotation);
}

Triangle Triangle::rotateZ(float angle) {
    Matrix rotation = Matrix::createRotationZ(angle);
    return transform(rotation);
}

Triangle Triangle::transform(const Matrix& matrix) {
    Verticle_[0] = Verticle_[0].Transform(matrix);
    Verticle_[1] = Verticle_[1].Transform(matrix);
    Verticle_[2] = Verticle_[2].Transform(matrix);
    return *this;
}

} // namespace
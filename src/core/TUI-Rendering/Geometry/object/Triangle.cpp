#include "../../../../include/TUI-Rendering/Geometry/object/Triangle.hpp"

namespace Geometry {

Triangle::Triangle(Vector A, Vector B, Vector C) : A_{A}, B_{B}, C_{C}, color_{TUI::Pixel{255, 255, 255, 255}} {}

Triangle::Triangle(Vector A, Vector B, Vector C, TUI::Pixel color) : A_{A}, B_{B}, C_{C}, color_{color} {}

Vector Triangle::getA() const { return A_; }
Vector Triangle::getB() const { return B_; }
Vector Triangle::getC() const { return C_; }
TUI::Pixel Triangle::getColor() const { return color_; }

Triangle Triangle::translate(float x, float y, float z) const {
    Matrix translation = Matrix::createTranslation(x, y, z);
    return transform(translation);
}

Triangle Triangle::rotateX(float angle) const {
    Matrix rotation = Matrix::createRotationX(angle);
    return transform(rotation);
}

Triangle Triangle::rotateY(float angle) const {
    Matrix rotation = Matrix::createRotationY(angle);
    return transform(rotation);
}

Triangle Triangle::rotateZ(float angle) const {
    Matrix rotation = Matrix::createRotationZ(angle);
    return transform(rotation);
}

Triangle Triangle::transform(const Matrix& matrix) const {
    return Triangle(
        A_.Transform(matrix),
        B_.Transform(matrix),
        C_.Transform(matrix),
        color_
    );
}

} // namespace
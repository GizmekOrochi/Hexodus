#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"
#include "../../../TUIRendering.hpp"

namespace Geometry {

class Triangle {
private:
    Vector A_;
    Vector B_;
    Vector C_;
    TUI::Pixel color_;
public:
    Triangle(Vector A, Vector B, Vector C);
    Triangle(Vector A, Vector B, Vector C, TUI::Pixel color);

    Vector getA() const;
    Vector getB() const;
    Vector getC() const;
    TUI::Pixel getColor() const;
    
    Triangle translate(float x, float y, float z) const;
    Triangle rotateX(float angle) const;
    Triangle rotateY(float angle) const;
    Triangle rotateZ(float angle) const;
    Triangle transform(const Matrix& matrix) const;
};

}

#endif // TRIANGLE_HPP

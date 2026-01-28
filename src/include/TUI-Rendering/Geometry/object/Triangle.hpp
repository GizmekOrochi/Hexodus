#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#pragma once

#include <array>
#include <cstdint>
#include "Vector.hpp"
#include "Matrix.hpp"
#include "../../../TUIRendering.hpp"

namespace Geometry {

class Triangle {
private:
    std::array<Vector, 3> Verticle_;
    std::array<TUI::Pixel, 3> color_;
    // gradiant color %
    std::array<uint8_t, 3> gradiant_;
public:
    Triangle(Vector A, Vector B, Vector C);
    Triangle(Vector A, Vector B, Vector C, TUI::Pixel colorA, TUI::Pixel colorB, TUI::Pixel colorC);
    Triangle(Vector A, Vector B, Vector C, TUI::Pixel colorA, TUI::Pixel colorB, TUI::Pixel colorC, uint8_t gradiantA, uint8_t gradiantB, uint8_t gradiantC);
    Triangle(const Triangle& other);

    Vector getA() const;
    Vector getB() const;
    Vector getC() const;
    TUI::Pixel getColorA() const;
    TUI::Pixel getColorB() const;
    TUI::Pixel getColorC() const;
    uint8_t getGradiantA() const;
    uint8_t getGradiantB() const;
    uint8_t getGradiantC() const;

    Triangle translate(float x, float y, float z) const;
    Triangle rotateX(float angle) const;
    Triangle rotateY(float angle) const;
    Triangle rotateZ(float angle) const;
    Triangle transform(const Matrix& matrix) const;
};

}

#endif // TRIANGLE_HPP

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

    Triangle transform(const Matrix& matrix);
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

    Triangle translate(float x, float y, float z);
    Triangle translate(float x, float y, float z, const Vector& center);
    
    Triangle rotateX(float angle);
    Triangle rotateX(float angle, const Vector& center);

    Triangle rotateY(float angle);
    Triangle rotateY(float angle, const Vector& center);

    Triangle rotateZ(float angle);
    Triangle rotateZ(float angle, const Vector& center);
};

}

#endif // TRIANGLE_HPP

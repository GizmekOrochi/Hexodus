#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#pragma once

#include "Vector.hpp"

namespace Geometry {

class Triangle {
private:
    Vector A_;
    Vector B_;
    Vector C_;
public:
    Triangle(Vector A, Vector B, Vector C);

    Vector getA();
    Vector getB();
    Vector getC();
};

}

#endif // TRIANGLE_HPP

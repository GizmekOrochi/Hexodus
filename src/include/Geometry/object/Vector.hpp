#ifndef VECTOR_HPP
#define VECTOR_HPP

#pragma once

#include <cmath>

namespace Geometry {

class Vector {
private:
    float x_{};
    float y_{};
    float z_{};
    float w_{};   // weight for mass point algebra
public:
    Vector(float x, float y, float z, bool isVector);
    Vector(float x, float y, float z, float w);
    Vector();
    ~Vector();

    float getX() const;
    float getY() const;
    float getZ() const;
    float getW() const;

    void operator=(Vector& other);

    void operator+=(Vector& other);
    void operator-=(Vector& other);

    Vector operator+(Vector& other);
    Vector operator-(Vector& other);

    float Norm();
    float Dot(Vector& other);
};

} //namespace

#endif // VECTOR_HPP

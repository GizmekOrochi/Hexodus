#ifndef VECTOR_HPP
#define VECTOR_HPP

#pragma once
#include <cmath>
#include <stdexcept>
#include "Matrix.hpp"

namespace Geometry {

class Vector {
private:
    float x_{};
    float y_{};
    float z_{};
    float w_{};
public:
    Vector(float x, float y, float z, bool isVector);
    Vector(float x, float y, float z, float w);
    Vector(const Vector& copy);
    Vector();

    float getX() const;
    float getY() const;
    float getZ() const;
    float getW() const;

    Vector& operator=(const Vector& other);

    Vector& operator+=(const Vector& other);
    Vector& operator-=(const Vector& other);
    Vector& operator*=(float scalar);
    Vector& operator/=(float scalar);

    Vector operator+(const Vector& other) const;
    Vector operator-(const Vector& other) const;
    Vector operator*(float scalar) const;
    Vector operator/(float scalar) const;

    friend Vector operator*(const Matrix& matrix, const Vector& vector);
    friend Vector operator*(const Vector& vector, const Matrix& matrix);

    float Norm() const;
    float Dot(const Vector& other) const;
    Vector Cross(const Vector& other) const;

    float lengthSq() const;
    float length() const;

    Vector getRealPosition() const;

    Vector Rotate(const Vector& vec) const;
    Vector Translate(const Vector& vec) const;
    Vector Transform(const Vector& vec) const;
    Vector Transform(const Matrix& transformMat) const;
};

} //namespace

#endif // VECTOR_HPP
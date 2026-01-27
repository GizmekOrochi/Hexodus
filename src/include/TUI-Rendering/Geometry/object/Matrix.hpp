#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <array>
#include <cstddef>
#include <stdexcept>
#include <cmath>

namespace Geometry {

class Matrix {
private:
    std::array<float, 16> value_;
public:
    // Default constructor == identity matrix
    Matrix();
    Matrix(std::array<float, 16> value);

    std::array<float, 16> getValue() const;

    static Matrix Identity();

    Matrix& operator=(const Matrix& other);

    float& operator[](std::size_t index);
    const float& operator[](std::size_t index) const;

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const;

    Matrix operator*(float scalar) const;
    Matrix operator/(float scalar) const;

    Matrix& operator+=(const Matrix& other);
    Matrix& operator-=(const Matrix& other);
    Matrix& operator*=(float scalar);
    Matrix& operator/=(float scalar);

    bool operator==(const Matrix& other) const;
    bool operator!=(const Matrix& other) const;

    Matrix transpose() const;
    static Matrix createTranslation(float x, float y, float z);
    static Matrix createScale(float x, float y, float z);
    static Matrix createRotationX(float angle);
    static Matrix createRotationY(float angle);
    static Matrix createRotationZ(float angle);
};

} // namespace

#endif // MATRIX_HPP
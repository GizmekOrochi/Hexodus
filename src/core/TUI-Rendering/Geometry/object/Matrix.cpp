#include "../../../../include/TUI-Rendering/Geometry/object/Matrix.hpp"

namespace Geometry {

Matrix::Matrix() : value_({
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    }) {}

Matrix::Matrix(std::array<float, 16> value) : value_(value) {}

std::array<float, 16> Matrix::getValue() const { return this->value_; };

Matrix Matrix::Identity() {
    return Matrix();
}

Matrix& Matrix::operator=(const Matrix& other) {
    if (this != &other) {
        value_ = other.value_;
    }
    return *this;
}

float& Matrix::operator[](std::size_t index) {
    if (index >= 16) {
        throw std::out_of_range("Matrix index out of range");
    }
    return value_[index];
}

const float& Matrix::operator[](std::size_t index) const {
    if (index >= 16) {
        throw std::out_of_range("Matrix index out of range");
    }
    return value_[index];
}

Matrix Matrix::operator+(const Matrix& other) const {
    std::array<float, 16> res{value_};
    for (std::size_t i = 0; i < 16; ++i) {
        res[i] += other.value_[i];
    }
    return Matrix{res};
}

Matrix Matrix::operator-(const Matrix& other) const {
    std::array<float, 16> res{value_};
    for (std::size_t i = 0; i < 16; ++i) {
        res[i] -= other.value_[i];
    }
    return Matrix{res};
}

Matrix Matrix::operator*(float scalar) const {
    std::array<float, 16> res{value_};
    for (std::size_t i = 0; i < 16; ++i) {
        res[i] *= scalar;
    }
    return Matrix{res};
}

Matrix Matrix::operator/(float scalar) const {
    if (scalar == 0.0f) {
        throw std::out_of_range("Division by 0");
    }
    std::array<float, 16> res{value_};
    for (std::size_t i = 0; i < 16; ++i) {
        res[i] /= scalar;
    }
    return Matrix{res};
}

Matrix Matrix::operator*(const Matrix& other) const {
    std::array<float, 16> res{};
    const auto& a = value_;
    const auto& b = other.value_;

    for (int i = 0; i < 4; ++i) {
        const int i4 = i * 4;
        res[i4 + 0] = a[i4 + 0] * b[0] + a[i4 + 1] * b[4] + a[i4 + 2] * b[8] + a[i4 + 3] * b[12];
        res[i4 + 1] = a[i4 + 0] * b[1] + a[i4 + 1] * b[5] + a[i4 + 2] * b[9] + a[i4 + 3] * b[13];
        res[i4 + 2] = a[i4 + 0] * b[2] + a[i4 + 1] * b[6] + a[i4 + 2] * b[10] + a[i4 + 3] * b[14];
        res[i4 + 3] = a[i4 + 0] * b[3] + a[i4 + 1] * b[7] + a[i4 + 2] * b[11] + a[i4 + 3] * b[15];
    }
    return Matrix{res};
}

Matrix& Matrix::operator+=(const Matrix& other) {
    for (std::size_t i = 0; i < 16; ++i) {
        value_[i] += other.value_[i];
    }
    return *this;
}

Matrix& Matrix::operator-=(const Matrix& other) {
    for (std::size_t i = 0; i < 16; ++i) {
        value_[i] -= other.value_[i];
    }
    return *this;
}

Matrix& Matrix::operator*=(float scalar) {
    for (std::size_t i = 0; i < 16; ++i) {
        value_[i] *= scalar;
    }
    return *this;
}

Matrix& Matrix::operator/=(float scalar) {
    if (scalar != 0.0f) {
        for (std::size_t i = 0; i < 16; ++i) {
            value_[i] /= scalar;
        }
    }
    return *this;
}

bool Matrix::operator==(const Matrix& other) const {
    for (std::size_t i = 0; i < 16; ++i) {
        if (value_[i] != other.value_[i]) {
            return false;
        }
    }
    return true;
}

bool Matrix::operator!=(const Matrix& other) const {
    return !(*this == other);
}

Matrix Matrix::transpose() const {
    std::array<float, 16> res{};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            res[col * 4 + row] = value_[row * 4 + col];
        }
    }
    return Matrix{res};
}

Matrix Matrix::createTranslation(float x, float y, float z) {
    std::array<float, 16> res = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        x, y, z, 1.f
    };
    return Matrix{res};
}

Matrix Matrix::createScale(float x, float y, float z) {
    std::array<float, 16> res = {
        x, 0.f, 0.f, 0.f,
        0.f, y, 0.f, 0.f,
        0.f, 0.f, z, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    return Matrix{res};
}

Matrix Matrix::createRotationX(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    std::array<float, 16> res = {
        1.f, 0.f, 0.f, 0.f,
        0.f, c, -s, 0.f,
        0.f, s, c, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    return Matrix{res};
}

Matrix Matrix::createRotationY(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    std::array<float, 16> res = {
        c, 0.f, s, 0.f,
        0.f, 1.f, 0.f, 0.f,
        -s, 0.f, c, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    return Matrix{res};
}

Matrix Matrix::createRotationZ(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    std::array<float, 16> res = {
        c, -s, 0.f, 0.f,
        s, c, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    return Matrix{res};
}

} // namespace Geometry
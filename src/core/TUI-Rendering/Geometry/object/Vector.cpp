#include "../../../../include/TUI-Rendering/Geometry/object/Vector.hpp"

namespace Geometry {

Vector::Vector() : x_(0.0f), y_(0.0f), z_(0.0f), w_(1.0f) {}
Vector::Vector(float x, float y, float z, bool isVector) : x_(x), y_(y), z_(z) {
    w_ = (isVector) ? 0.f : 1.f;
};
Vector::Vector(const Vector& copy) : x_(copy.getX()), y_(copy.getY()), z_(copy.getZ()), w_(copy.getW()) {
};

Vector::Vector(float x, float y, float z, float w) : x_(x), y_(y), z_(z), w_(w) {};

float Vector::getX() const { return x_; } 
float Vector::getY() const { return y_; } 
float Vector::getZ() const { return z_; } 
float Vector::getW() const { return w_; } 

Vector& Vector::operator=(const Vector& other) {
    if (this != &other) {
        this->x_ = other.getX();
        this->y_ = other.getY();
        this->z_ = other.getZ();
        this->w_ = other.getW();
    }
    return *this;
}

Vector& Vector::operator+=(const Vector& other) {
    this->x_ += other.getX();
    this->y_ += other.getY();
    this->z_ += other.getZ();
    return *this;
}

Vector& Vector::operator-=(const Vector& other) {
    this->x_ -= other.getX();
    this->y_ -= other.getY();
    this->z_ -= other.getZ();
    return *this;
}

Vector& Vector::operator*=(float scalar) {
    this->x_ *= scalar;
    this->y_ *= scalar;
    this->z_ *= scalar;
    return *this;
}

Vector& Vector::operator/=(float scalar) {
    if (scalar != 0.0f) {
        this->x_ /= scalar;
        this->y_ /= scalar;
        this->z_ /= scalar;
    }
    return *this;
}

Vector Vector::operator+(const Vector& other) const {
    if(this->w_ == 1.0f && other.getW() == 1.0f) throw std::out_of_range("Point - Point mean nothing");
    float newVecX = this->x_ + other.getX();
    float newVecY = this->y_ + other.getY();
    float newVecZ = this->z_ + other.getZ();
    float newW = (this->w_ == 0.0f && other.getW() == 0.0f) ? 0.0f : 1.0f;
    return Vector{newVecX, newVecY, newVecZ, newW};
}

Vector Vector::operator-(const Vector& other) const {
    if(this->w_ == 1.0f && other.getW() == 1.0f) throw std::out_of_range("Point - Point mean nothing");
    float newVecX = this->x_ - other.getX();
    float newVecY = this->y_ - other.getY();
    float newVecZ = this->z_ - other.getZ();
    float newW = (this->w_ == 0.0f && other.getW() == 0.0f) ? 0.0f : 1.0f;
    return Vector{newVecX, newVecY, newVecZ, newW};
}

Vector Vector::operator*(float scalar) const { 
    float newVecX = this->x_ * scalar;
    float newVecY = this->y_ * scalar;
    float newVecZ = this->z_ * scalar;
    return Vector{newVecX, newVecY, newVecZ, this->w_};
}
      
Vector Vector::operator/(float scalar) const { 
    if (scalar == 0.0f) throw std::out_of_range("Division by 0");
    float newVecX = this->x_ / scalar;
    float newVecY = this->y_ / scalar;
    float newVecZ = this->z_ / scalar;
    return Vector{newVecX, newVecY, newVecZ, this->w_};
}

Vector operator*(const Matrix& matrix, const Vector& vector) {
    float x = vector.getX() * matrix[0] + vector.getY() * matrix[4] + vector.getZ() * matrix[8] + vector.getW() * matrix[12];
    float y = vector.getX() * matrix[1] + vector.getY() * matrix[5] + vector.getZ() * matrix[9] + vector.getW() * matrix[13];
    float z = vector.getX() * matrix[2] + vector.getY() * matrix[6] + vector.getZ() * matrix[10] + vector.getW() * matrix[14];
    float w = vector.getX() * matrix[3] + vector.getY() * matrix[7] + vector.getZ() * matrix[11] + vector.getW() * matrix[15];
    return Vector(x, y, z, w);
}

Vector operator*(const Vector& vector, const Matrix& matrix) {
    float x = vector.getX() * matrix[0] + vector.getY() * matrix[1] + vector.getZ() * matrix[2] + vector.getW() * matrix[3];
    float y = vector.getX() * matrix[4] + vector.getY() * matrix[5] + vector.getZ() * matrix[6] + vector.getW() * matrix[7];
    float z = vector.getX() * matrix[8] + vector.getY() * matrix[9] + vector.getZ() * matrix[10] + vector.getW() * matrix[11];
    float w = vector.getX() * matrix[12] + vector.getY() * matrix[13] + vector.getZ() * matrix[14] + vector.getW() * matrix[15];
    return Vector(x, y, z, w);
}

float Vector::Norm() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}

float Vector::Dot(const Vector& other) const {
    return (x_ * other.getX() + y_ * other.getY() + z_ * other.getZ());
}

Vector Vector::Cross(const Vector& other) const {
    return Vector{
        y_ * other.getZ() - z_ * other.getY(),
        z_ * other.getX() - x_ * other.getZ(),
        x_ * other.getY() - y_ * other.getX(),
        0.f
    };
}

float Vector::lengthSq() const {
    return x_ * x_ + y_ * y_ + z_ * z_;
}

float Vector::length() const { 
    return std::sqrt(lengthSq());
}

Vector Vector::getRealPosition() const {
    if (w_ == 0.0f) {
        throw std::out_of_range("Don't work on Vectors");
    }
    return Vector{x_ / w_, y_ / w_, z_ / w_, 1.0f};
}

Vector Vector::Rotate() const {
    // Implementation depends on your rotation requirements
    return *this;
}

Vector Vector::Translate() const {
    // Implementation depends on your translation requirements
    return *this;
}

Vector Vector::Transform() const {
    // Implementation depends on your transformation requirements
    return *this;
}

} // namespace
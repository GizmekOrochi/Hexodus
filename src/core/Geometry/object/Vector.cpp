#include "../../../include/Geometry/object/Vector.hpp"

namespace Geometry {

Vector::Vector(float x, float y, float z, bool isVector) : x_(x), y_(y), z_(z) {
    w_ = (isVector) ? 0.f : 1.f;
};

Vector::Vector(float x, float y, float z, float w) : x_(x), y_(y), z_(z), w_(w) {};

float Vector::getX() const { return x_; } 
float Vector::getY() const { return y_; } 
float Vector::getZ() const { return z_; } 
float Vector::getW() const { return w_; } 

void Vector::operator=(Vector& other) {
    this->x_ = other.getX();
    this->y_ = other.getY();
    this->z_ = other.getZ();
    this->w_ = other.getW();
}

void Vector::operator+=(Vector& other) {
    this->x_ += other.getX();
    this->y_ += other.getY();
    this->z_ += other.getZ();
}

void Vector::operator-=(Vector& other) {
    this->x_ -= other.getX();
    this->y_ -= other.getY();
    this->z_ -= other.getZ();
}

Vector Vector::operator+(Vector& other) {
    float newVecX{this->x_ + other.getX()};
    float newVecY{this->x_ + other.getX()};
    float newVecZ{this->x_ + other.getX()};
    return Vector{newVecX, newVecY, newVecZ, 1.f};
};

Vector Vector::operator-(Vector& other) {
    float newVecX{this->x_ - other.getX()};
    float newVecY{this->x_ - other.getX()};
    float newVecZ{this->x_ - other.getX()};
    return Vector{newVecX, newVecY, newVecZ, 1.f};
};

float Vector::Norm() {
    return std::sqrt(std::pow(this->x_, 2) + std::pow(this->y_, 2) + std::pow(this->z_, 2));
};

float Vector::Dot(Vector& other) {
    return (this->x_ * other.getX() + this->y_ * other.getY() + this->z_ * other.getZ());

};

}
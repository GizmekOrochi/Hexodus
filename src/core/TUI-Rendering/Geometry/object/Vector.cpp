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

void Vector::setX(const float& value) { this->x_ = value; } 
void Vector::setY(const float& value) { this->y_ = value; } 
void Vector::setZ(const float& value) { this->z_ = value; } 
void Vector::setW(const float& value) { this->w_ = value; } 

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
    *this = *this + other;
    return *this;
}

Vector& Vector::operator-=(const Vector& other) {
    *this = *this - other;
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
    if (w_ == 0.0f && other.w_ == 0.0f) return Vector{x_ + other.x_, y_ + other.y_, z_ + other.z_, 0.0f};
    if (w_ == 1.0f && other.w_ == 0.0f) return Vector{x_ + other.x_, y_ + other.y_, z_ + other.z_, 1.0f};
    if (w_ == 0.0f && other.w_ == 1.0f) throw std::out_of_range("Vector + Point is undefined");
    throw std::out_of_range("Point + Point is undefined");
}


Vector Vector::operator-(const Vector& other) const {
    if (w_ == 1.0f && other.w_ == 1.0f) return Vector{x_ - other.x_, y_ - other.y_, z_ - other.z_, 0.0f};
    if (w_ == 1.0f && other.w_ == 0.0f) return Vector{x_ - other.x_, y_ - other.y_, z_ - other.z_, 1.0f};
    if (w_ == 0.0f && other.w_ == 0.0f) return Vector{x_ - other.x_, y_ - other.y_, z_ - other.z_, 0.0f};
    throw std::out_of_range("Vector - Point is undefined");
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

Vector operator*(const Vector& vector, const Matrix& matrix) {
    float x = vector.getX() * matrix[0] + vector.getY() * matrix[4] + vector.getZ() * matrix[8] + vector.getW() * matrix[12];
    float y = vector.getX() * matrix[1] + vector.getY() * matrix[5] + vector.getZ() * matrix[9] + vector.getW() * matrix[13];
    float z = vector.getX() * matrix[2] + vector.getY() * matrix[6] + vector.getZ() * matrix[10] + vector.getW() * matrix[14];
    float w = vector.getX() * matrix[3] + vector.getY() * matrix[7] + vector.getZ() * matrix[11] + vector.getW() * matrix[15];
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

Vector Vector::Rotate(const Vector& vec) const {
    Matrix rotX = Matrix::createRotationX(vec.getX());
    Matrix rotY = Matrix::createRotationY(vec.getY());
    Matrix rotZ = Matrix::createRotationZ(vec.getZ());

    Geometry::Matrix rotationMatrix = rotZ * rotY * rotX;
    return Transform(rotationMatrix);
}

Vector Vector::Translate(const Vector& vec) const {
    Matrix translationMatrix = Matrix::createTranslation(vec.getX(), vec.getY(), vec.getZ());

    return Transform(translationMatrix);
}

Vector Vector::Transform(const Matrix& transformMat) const {
    const std::array<float, 16>& m = transformMat.getValue();
    
    float newX = x_ * m[0] + y_ * m[4] + z_ * m[8] + w_ * m[12];
    float newY = x_ * m[1] + y_ * m[5] + z_ * m[9] + w_ * m[13];
    float newZ = x_ * m[2] + y_ * m[6] + z_ * m[10] + w_ * m[14];
    float newW = x_ * m[3] + y_ * m[7] + z_ * m[11] + w_ * m[15];
    if (newW != 1.0f && newW != 0.0f) {
        newX /= newW;
        newY /= newW;
        newZ /= newW;
        newW = 1.0f;
    }
    
    return Vector(newX, newY, newZ, newW);
}

Vector Vector::Transform(const Vector& vec) const {
    Matrix translation = Matrix::createTranslation(vec.getX(), vec.getY(), vec.getZ());

    return Transform(translation);
}

} // namespace
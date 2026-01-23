#include "../../include/TUI-Rendering/Camera.hpp"

namespace Rendering {

Camera::Camera() = default;

float Camera::getYaw() const {
    return rotation.getX();
}

float Camera::getPitch() const {
    return rotation.getY();
}

void Camera::moveForward(float dt) {
    Vector forwardVec = forward();
    position += forwardVec * (moveSpeed * dt);
}

void Camera::moveRight(float dt) {
    Vector rightVec = right();
    position += rightVec * (moveSpeed * dt);
}

void Camera::moveUp(float dt) {
    Vector upVec{0.0f, 1.0f, 0.0f, 0.0f};
    position += upVec * (moveSpeed * dt);
}

void Camera::rotate(float dyaw, float dpitch) {
    Vector currentRotation = rotation;
    Vector newRotation(currentRotation.getX() + dyaw * rotSpeed, currentRotation.getY() + dpitch * rotSpeed, currentRotation.getZ(), currentRotation.getW());
    rotation = newRotation;
    clampPitch();
}

void Camera::clampPitch() {
    float pitch = rotation.getY();
    pitch = std::clamp(pitch, -1.45f, 1.45f);
    Vector newRotation(rotation.getX(), pitch, rotation.getZ(), rotation.getW());
    rotation = newRotation;
}

const Vector& Camera::getPosition() const {
    return position;
}

Vector Camera::forward() const {
    float cy{std::cos(rotation.getX())}; 
    float sy{std::sin(rotation.getX())};
    float cp{std::cos(rotation.getY())};
    float sp{std::sin(rotation.getY())};
    return Vector{sy * cp, -sp, cy * cp, 0.f};
}

Vector Camera::right() const {
    float cy = std::cos(rotation.getX());
    float sy = std::sin(rotation.getX());
    return Vector{cy, 0.0f, -sy, 0.f};
}

Vector Camera::up() const {
    return Vector{0.0f, 1.0f, 0.0f, 0.f};
}

} // namespace Rendering
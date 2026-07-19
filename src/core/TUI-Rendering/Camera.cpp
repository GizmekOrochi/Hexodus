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
    const float yaw{rotation.getX()};

    const Vector horizontalForward{std::sin(yaw), 0.0f, std::cos(yaw), 0.0f};

    position += horizontalForward * (moveSpeed * dt);
}

void Camera::moveRight(float dt) {
    const float yaw{rotation.getX()};

    const Vector horizontalRight{std::cos(yaw), 0.0f, -std::sin(yaw), 0.0f};

    position += horizontalRight * (moveSpeed * dt);
}

void Camera::moveUp(float dt) {
    const Vector worldUp{0.0f, 1.0f, 0.0f, 0.0f};

    position += worldUp * (moveSpeed * dt);
}

void Camera::rotate(float dyaw, float dpitch) {
    rotation = Vector{rotation.getX() + dyaw * rotSpeed, rotation.getY() + dpitch * rotSpeed, rotation.getZ(), rotation.getW()};

    clampPitch();
}

void Camera::clampPitch() {
    const float clampedPitch{std::clamp(rotation.getY(), -1.45f, 1.45f)};

    rotation = Vector{rotation.getX(), clampedPitch, rotation.getZ(), rotation.getW()};
}

const Vector& Camera::getPosition() const {
    return position;
}

void Camera::setPosition(const Vector& pos) {
    position = pos;
}

void Camera::lookAt(const Vector& target) {
    Vector direction{target - position};

    const float directionLength{direction.length()};

    if (directionLength <= 1e-6f) {
        return;
    }

    direction /= directionLength;

    const float yaw{std::atan2(direction.getX(), direction.getZ())};
    const float pitch{-std::asin(std::clamp(direction.getY(), -1.0f, 1.0f))};

    rotation = Vector{yaw, pitch, 0.0f, 0.0f};

    clampPitch();
}

Vector Camera::forward() const {
    const float yaw{rotation.getX()};
    const float pitch{rotation.getY()};

    const float cosYaw{std::cos(yaw)};
    const float sinYaw{std::sin(yaw)};
    const float cosPitch{std::cos(pitch)};
    const float sinPitch{std::sin(pitch)};

    return Vector{sinYaw * cosPitch, -sinPitch, cosYaw * cosPitch, 0.0f};
}

Vector Camera::right() const {
    const float yaw{rotation.getX()};

    return Vector{std::cos(yaw), 0.0f, -std::sin(yaw), 0.0f};
}

Vector Camera::up() const {
    const Vector forwardDirection{forward()};
    const Vector rightDirection{right()};

    return forwardDirection.Cross(rightDirection);
}

void Camera::move(const Vector& target, const Vector& direction, float deltaTime) {
    Vector dir{direction};

    const float length{dir.length()};

    if (length <= 1e-6f) {
        return;
    }

    dir /= length;

    position = target + dir * (moveSpeed * deltaTime);
}

} // namespace Rendering
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <algorithm>
#include <cmath>
#include "Geometry/object/Vector.hpp"

namespace Rendering {

using namespace Geometry;

class Camera {
public:
    Camera();

    void moveForward(float dt);
    void moveRight(float dt);
    void moveUp(float dt);

    void rotate(float deltaYaw, float deltaPitch);

    const Vector& getPosition() const;
    void setPosition(const Vector& pos);

    void lookAt(const Vector& target);

    float getYaw() const;
    float getPitch() const;

    Vector forward() const;
    Vector right() const;
    Vector up() const;

private:
    Vector position{0.0f, 0.0f, -250.0f, 1.f};
    Vector rotation{};

    float moveSpeed{200.0f};
    float rotSpeed{0.5f};

    void clampPitch();
};

} // namespace Rendering

#endif
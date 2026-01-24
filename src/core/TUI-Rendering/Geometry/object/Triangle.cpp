#include "../../../../include/TUI-Rendering/Geometry/object/Triangle.hpp"

namespace Geometry {

Triangle::Triangle(Vector A, Vector B, Vector C) : A_{A}, B_{B}, C_{C} {}

Vector Triangle::getA() { return A_; };
Vector Triangle::getB() { return B_; };
Vector Triangle::getC() { return C_; };
}
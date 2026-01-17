#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <string>
#include "../GeometryStruct.hpp"

namespace Geometry {

class Triangle {
private:
    Verticals Vertical_A;
    Verticals Vertical_B;
    Verticals Vertical_C;
public:
    Triangle();
    ~Triangle();
};

} // namespace

#endif // TRIANGLE_HPP

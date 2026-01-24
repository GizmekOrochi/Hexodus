#ifndef GEOMETRICALSTRUCT_HPP
#define GEOMETRICALSTRUCT_HPP

#pragma once

namespace Geometry {

struct Verticals {
    float X_, Y_;
};

struct Segment {
    Verticals vertical_1;
    Verticals vertical_2;
};

struct Area {
    Segment segment;
    bool extruded;
};

struct Shape {
    /* data */
};

} //namespace

#endif
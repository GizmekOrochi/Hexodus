#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#include <string>
#include <vector>

namespace Geometry {

class Rasterizer {
private:
    float worldX_Limit_;
    float worldY_Limit_;
public:
    Rasterizer(float worldX_Limit, float worldY_Limit) : worldX_Limit_(worldX_Limit), worldY_Limit_(worldY_Limit) {};
    ~Rasterizer();
};

}

#endif // RASTERIZER_HPP

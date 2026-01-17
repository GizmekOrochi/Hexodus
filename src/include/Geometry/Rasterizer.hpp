#ifndef TUIRASTERIZER_HPP
#define TUIRASTERIZER_HPP

#include <string>
#include <vector>

namespace TUI {

class TUIRasterizer {
private:
    float worldX_Limit_;
    float worldY_Limit_;
public:
    TUIRasterizer(float worldX_Limit, float worldY_Limit) : worldX_Limit_(worldX_Limit), worldY_Limit_(worldY_Limit) {};
    ~TUIRasterizer();
};

}

#endif // TUIRASTERIZER_HPP

#ifndef TUIRENDERER_HPP
#define TUIRENDERER_HPP

#include <cstdint>
#include <vector>

struct Pixel {
    uint8_t r, g, b, a;
};

struct Verticals {
    uint16_t X_, Y_;
};


class TUIRenderer {
private:
    std::vector<Pixel> framebuffer;
    int height_;
    int width_;
public:
    TUIRenderer() : height_(), width_() {};

    void setBackgroundRed() {
        for(size_t y{}; y < width_; y++){
            std::vector<std::vector<Pixel>> newTab;
            for(size_t x{}; x < height_; x++){
                framebuffer.push_back(Pixel{255, 0, 0, 255});
            }
        }
    };

    void setBufferSize(int h, int w) {
        height_ = h;
        width_ = w;
    }
    
    void drawBuffer() {

    }
};

#endif // TUIRENDERER_HPP

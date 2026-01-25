#ifndef SCENE_HPP
#define SCENE_HPP

#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include "../TUIRendering.hpp"

class Scene {
private:
    // locations are a % of the total screen no need more that uint8_t
    std::array<uint8_t, 2> origin_;
    std::array<uint8_t, 2> ending_;
    TUI::Pixel color_;
public:
    Scene(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending);
    virtual ~Scene() = default;
    void setBackground(TUI::Pixel color);

    TUI::Pixel Color();
    std::array<uint8_t, 2> Origin();
    std::array<uint8_t, 2> Ending();

    void Origin(const std::array<uint8_t, 2>& origin);
    void Ending(const std::array<uint8_t, 2>& ending);

    virtual std::vector<TUI::Pixel> convertScene(int outputHeight, int outputLength) = 0;
};

#endif // SCENE_HPP

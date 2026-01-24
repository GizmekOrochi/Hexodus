#ifndef Scene2D_HPP
#define Scene2D_HPP

#pragma once
#include <array>
#include <vector>
#include <memory>
#include <cstdint>
#include <climits>
#include "Scene.hpp"
#include "UI/UI.hpp"

class Scene2D : public Scene {
private:
    std::vector<std::unique_ptr<UIElement>> SceneElements_;
public:
    Scene2D(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending);

    UIElement& Element(size_t index);
    const UIElement& Element(size_t index) const;
    void add(std::unique_ptr<UIElement> element);

    std::vector<TUI::Pixel> convertScene(int outputHeight, int outputLenght);
};

#endif // Scene2D_HPP

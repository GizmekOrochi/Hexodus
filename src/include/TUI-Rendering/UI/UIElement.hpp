#ifndef UIELEMENT_HPP
#define UIELEMENT_HPP

#pragma once
#include <array>

namespace UI {

class UIElement {
private: 
    // locations are a 1 per 1000 of the scene size
    std::array<int, 2> origin_;
    std::array<int, 2> ending_;
    int importance_;
public:
    UIElement(std::array<int, 2> origin, std::array<int, 2> ending, int importance);
    UIElement(const UIElement& str);

    virtual ~UIElement() = default;

    int Importance() const;
    std::array<int, 2> Origin() const;
    std::array<int, 2> Ending() const;

    void Importance(const int& importance);
    void Origin(const std::array<int, 2>& origin);
    void Ending(const std::array<int, 2>& ending);
};

}

#endif // UIELEMENT_HPP

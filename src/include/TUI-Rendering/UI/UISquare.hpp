#ifndef UISQUARE_HPP
#define UISQUARE_HPP

#pragma once
#include <string>
#include <array>
#include "UIElement.hpp"
#include "../../TUIRendering.hpp"

namespace UI {

class UISquare : public UIElement {
private: 
    TUI::Pixel color_;
public:
    UISquare(std::array<int, 2> origin, std::array<int, 2> ending, int importance,TUI::Pixel color);
    UISquare(const UISquare& str);

    TUI::Pixel Color() const;

    void Color(const TUI::Pixel& color);
};

}

#endif // UISquare_HPP

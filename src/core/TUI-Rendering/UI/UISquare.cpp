#include "../../../include/TUI-Rendering/UI/UISquare.hpp"

namespace UI {

UISquare::UISquare(std::array<int, 2> origin, std::array<int, 2> ending, int importance, TUI::Pixel color) 
    : UIElement{origin, ending, importance}, color_{color} {}

UISquare::UISquare(const UISquare& str)
    : UIElement{str}, color_{str.Color()} {}

TUI::Pixel UISquare::Color() const { return this->color_; }

void UISquare::Color(const TUI::Pixel& color) { this->color_ = color; }

}   // namespace
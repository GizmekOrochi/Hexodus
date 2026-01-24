#include "../../../include/TUI-Rendering/UI/UIElement.hpp"

namespace UI {

UIElement::UIElement(std::array<int, 2> origin, std::array<int, 2> ending, int importance) 
    : origin_{origin}, ending_{ending}, importance_{importance} {}

UIElement::UIElement(const UIElement& str)
    : origin_{str.Origin()}, ending_{str.Ending()}, importance_{str.Importance()} {}

int UIElement::Importance() const { return this->importance_; }   
std::array<int, 2> UIElement::Origin() const { return this->origin_; }
std::array<int, 2> UIElement::Ending() const { return this->ending_; }

void UIElement::Importance(const int& importance) { this->importance_ = importance; }
void UIElement::Origin(const std::array<int, 2>& origin) { this->origin_ = origin; }
void UIElement::Ending(const std::array<int, 2>& ending) { this->ending_ = ending; }

}   // namespace
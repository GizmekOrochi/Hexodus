#include "../../../include/TUI-Rendering/UI/UIString.hpp"

namespace UI {

UIString::UIString(std::array<int, 2> origin, std::array<int, 2> ending, int importance, std::string contant)
    : UIElement{origin, ending, importance}, contant_{contant} {}

UIString::UIString(const UIString& str)
    : UIElement{str}, contant_{str.Contant()} {}

std::string UIString::Contant() const { return this->contant_; }

void UIString::Contant(const std::string& contant) { this->contant_ = contant; }  

} // namespace
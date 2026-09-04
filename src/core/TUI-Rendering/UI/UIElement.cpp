#include "../../../include/TUI-Rendering/UI/UIElement.hpp"

namespace UI {

UIElement::UIElement(std::array<int, 2> origin, std::array<int, 2> ending,int importance)
    : origin_{origin}, ending_{ending}, importance_{importance}, anchor_{UIAnchor::TopLeft} {}

UIElement::UIElement(std::array<int, 2> origin, std::array<int, 2> ending, int importance, UIAnchor anchor)
    : origin_{origin}, ending_{ending}, importance_{importance}, anchor_{anchor} {}

UIElement::UIElement(const UIElement& element)
    : origin_{element.Origin()}, ending_{element.Ending()}, importance_{element.Importance()}, anchor_{element.Anchor()} {}

int UIElement::Importance() const {
    return importance_;
}

std::array<int, 2> UIElement::Origin() const {
    return origin_;
}

std::array<int, 2> UIElement::Ending() const {
    return ending_;
}

UIAnchor UIElement::Anchor() const noexcept {
    return anchor_;
}

void UIElement::Importance(const int& importance) {
    importance_ = importance;
}

void UIElement::Origin(const std::array<int, 2>& origin) {
    origin_ = origin;
}

void UIElement::Ending(const std::array<int, 2>& ending) {
    ending_ = ending;
}

void UIElement::Anchor(UIAnchor anchor) noexcept {
    anchor_ = anchor;
}

UIBounds UIElement::resolvedBounds() const noexcept {
    const int width{ending_[0] - origin_[0]};
    const int height{ending_[1] - origin_[1]};

    int x{origin_[0]};
    int y{origin_[1]};

    switch (anchor_) {
    case UIAnchor::TopLeft:
        break;

    case UIAnchor::TopCenter:
        x -= width / 2;
        break;

    case UIAnchor::TopRight:
        x -= width;
        break;

    case UIAnchor::CenterLeft:
        y -= height / 2;
        break;

    case UIAnchor::Center:
        x -= width / 2;
        y -= height / 2;
        break;

    case UIAnchor::CenterRight:
        x -= width;
        y -= height / 2;
        break;

    case UIAnchor::BottomLeft:
        y -= height;
        break;

    case UIAnchor::BottomCenter:
        x -= width / 2;
        y -= height;
        break;

    case UIAnchor::BottomRight:
        x -= width;
        y -= height;
        break;
    }

    return UIBounds{{x, y}, {x + width, y + height}};
}

} // namespace UI

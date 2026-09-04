#ifndef TUI_UIELEMENT_HPP
#define TUI_UIELEMENT_HPP

#pragma once

#include "UIAnchor.hpp"

#include <array>

namespace UI {

struct UIBounds {
    std::array<int, 2> origin{};
    std::array<int, 2> ending{};
};

class UIElement {
private:
    // locations are a 1 per 1000 of the scene size
    std::array<int, 2> origin_{};
    std::array<int, 2> ending_{};
    int importance_{0};
    UIAnchor anchor_{UIAnchor::TopLeft};

public:
    UIElement(std::array<int, 2> origin, std::array<int, 2> ending, int importance);
    UIElement(std::array<int, 2> origin, std::array<int, 2> ending, int importance, UIAnchor anchor);
    UIElement(const UIElement& element);

    virtual ~UIElement() = default;

    [[nodiscard]] int Importance() const;
    [[nodiscard]] std::array<int, 2> Origin() const;
    [[nodiscard]] std::array<int, 2> Ending() const;
    [[nodiscard]] UIAnchor Anchor() const noexcept;

    void Importance(const int& importance);
    void Origin(const std::array<int, 2>& origin);
    void Ending(const std::array<int, 2>& ending);
    void Anchor(UIAnchor anchor) noexcept;

    /**
    Les coordonnées UI existantes restent en 0..1000.
    
    TopLeft conserve exactement l'ancien comportement :
    Origin = coin haut-gauche
    Ending = coin bas-droit

    Pour les autres anchors :
    Origin devient le point d'ancrage.
    La taille de l'élément reste Ending - Origin.
     
    Exemple :
        Origin = {500, 500}
        Ending = {700, 600}
        Anchor = Center
    Taille = 200 x 100
    Bounds résolus = {400, 450} -> {600, 550}
    **/
    [[nodiscard]] UIBounds resolvedBounds() const noexcept;
};

} // namespace UI

#endif // TUI_UIELEMENT_HPP

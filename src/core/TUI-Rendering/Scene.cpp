#include "../../include/TUI-Rendering/Scene.hpp"

Scene::Scene(std::array<uint8_t, 2> origin, std::array<uint8_t, 2> ending)
    : origin_{origin}, ending_{ending}, color_{} {}

void Scene::setBackground(TUI::Pixel color) {
    this->color_ = color;
}

TUI::Pixel Scene::Color() { return this->color_; }
std::array<uint8_t, 2> Scene::Origin() { return this->origin_; }
std::array<uint8_t, 2> Scene::Ending() { return this->ending_; }

void Scene::Origin(const std::array<uint8_t, 2>& origin) { this->origin_ = origin; }
void Scene::Ending(const std::array<uint8_t, 2>& ending) { this-> ending_ = ending; }
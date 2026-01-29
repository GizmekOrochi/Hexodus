#ifndef COLORATOR_HPP
#define COLORATOR_HPP

#pragma once
#include <string>
#include <array>
#include <signal.h>     // signals (later)
#include <format>
#include <cstdlib>
#include <cstring>
#include "../TUIStruct.hpp"

namespace TUI {

class Colorator {
private:
    RGBPanel RGBFront_;
    RGBPanel RGBBack_;
    TUIcolorModAvalible ColorMod_{};

public:
    RGBPanel getRGBFront();
    RGBPanel getRGBBack();
    
    std::string GetColorString() const;
    void SetRGBColor(RGBPanel colorFront, RGBPanel colorBack);
    void ApplyColor();
    
    Colorator();
};

} // namespace

#endif // COLORATOR_HPP

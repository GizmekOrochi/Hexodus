#ifndef TUIRENDERING_HPP
#define TUIRENDERING_HPP

#pragma once

#include <cstdint>
#include <string_view>

namespace TUI {

struct Pixel {
    std::uint8_t r, g, b, a;
};

struct Glyphs {
    static constexpr std::string_view empty = " ";
    static constexpr std::string_view full  = "█";

    // Half blocks
    static constexpr std::string_view upperHalf = "▀";
    static constexpr std::string_view lowerHalf = "▄";
    static constexpr std::string_view leftHalf  = "▌";
    static constexpr std::string_view rightHalf = "▐";

    // Quadrants
    static constexpr std::string_view tl = "▘";
    static constexpr std::string_view tr = "▝";
    static constexpr std::string_view bl = "▖";
    static constexpr std::string_view br = "▗";

    static constexpr std::string_view tl_br = "▚";
    static constexpr std::string_view tr_bl = "▞";

    static constexpr std::string_view tl_tr_bl = "▛";
    static constexpr std::string_view tl_tr_br = "▜";
    static constexpr std::string_view tl_bl_br = "▙";
    static constexpr std::string_view tr_bl_br = "▟";
};

} // namespace TUI

#endif // TUIRENDERING_HPP
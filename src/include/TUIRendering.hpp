#ifndef TUIRENDERING_HPP
#define TUIRENDERING_HPP

#pragma once
#include <cstdint>
#include <string_view>

namespace TUI {

struct Pixel {
    uint8_t r, g, b, a;
};

struct QuadBlock {
    static constexpr std::string_view empty = " ";

    static constexpr std::string_view tl = "▘";
    static constexpr std::string_view tr = "▝";
    static constexpr std::string_view bl = "▖";
    static constexpr std::string_view br = "▗";

    static constexpr std::string_view tl_tr = "▀";
    static constexpr std::string_view bl_br = "▄";

    static constexpr std::string_view tl_bl = "▌";
    static constexpr std::string_view tr_br = "▐";

    static constexpr std::string_view tl_br = "▚";
    static constexpr std::string_view tr_bl = "▞";

    static constexpr std::string_view tl_tr_bl = "▛";
    static constexpr std::string_view tl_tr_br = "▜";
    static constexpr std::string_view tl_bl_br = "▙";
    static constexpr std::string_view tr_bl_br = "▟";

    static constexpr std::string_view full = "█";
};

} // namespace

#endif // TUIRENDERING_HPP


namespace TUI {

struct QuadBlock {
    static constexpr char empty = ' ';   // 0000

    static constexpr char tl = '▘';   // 0001
    static constexpr char tr = '▝';   // 0010
    static constexpr char bl = '▖';   // 0100
    static constexpr char br = '▗';   // 1000

    static constexpr char tl_tr = '▀';   // 0011 (top half)
    static constexpr char bl_br = '▄';   // 1100 (bottom half)

    static constexpr char tl_bl = '▌';   // 0101 (left half)
    static constexpr char tr_br = '▐';   // 1010 (right half)

    static constexpr char tl_br = '▚';   // 1001
    static constexpr char tr_bl = '▞';   // 0110

    static constexpr char tl_tr_bl = '▛';   // 0111
    static constexpr char tl_tr_br = '▜';   // 1011
    static constexpr char tl_bl_br = '▙';   // 1101
    static constexpr char tr_bl_br = '▟';   // 1110

    static constexpr char full = '█';   // 1111
};

}
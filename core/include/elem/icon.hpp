#pragma once

#include "elem/text.hpp"

#include <array>
#include <string_view>

namespace bgui {

inline constexpr std::array nerd_font_icon_codepoints{
    char32_t{0xf00c}, char32_t{0xf00d}, char32_t{0xf002}, char32_t{0xf013},
    char32_t{0xf015}, char32_t{0xf030}, char32_t{0xf044},
    char32_t{0xf04b}, char32_t{0xf04c}, char32_t{0xf04d}, char32_t{0xf05a},
    char32_t{0xf060}, char32_t{0xf061}, char32_t{0xf062}, char32_t{0xf063},
    char32_t{0xf067}, char32_t{0xf068}, char32_t{0xf06e}, char32_t{0xf070},
    char32_t{0xf071}, char32_t{0xf078}, char32_t{0xf079},
    char32_t{0xf07b}, char32_t{0xf07c}, char32_t{0xf084}, char32_t{0xf093},
    char32_t{0xf0c5}, char32_t{0xf0c7}, char32_t{0xf0d7}, char32_t{0xf0da},
    char32_t{0xf0e7}, char32_t{0xf0eb}, char32_t{0xf0f6}, char32_t{0xf0f7},
    char32_t{0xf111}, char32_t{0xf121}, char32_t{0xf144}, char32_t{0xf188},
    char32_t{0xf17a}, char32_t{0xf17c}, char32_t{0xf1b2}, char32_t{0xf1f8},
    char32_t{0xf2ed}, char32_t{0xf65e}
};

class icon final : public text {
public:
    explicit icon(std::string_view name, float scale = 0.4f);
};

}

#include "elem/icon.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {

char32_t find_icon_glyph(const std::string_view name) {
    static const std::unordered_map<std::string_view, char32_t> glyphs{
        {"search", 0xf002},
        {"settings", 0xf013},
        {"home", 0xf015},
        {"camera", 0xf030},
        {"edit", 0xf044},
        {"play", 0xf04b},
        {"pause", 0xf04c},
        {"stop", 0xf04d},
        {"info", 0xf05a},
        {"arrow_left", 0xf060},
        {"arrow_right", 0xf061},
        {"arrow_up", 0xf062},
        {"arrow_down", 0xf063},
        {"eye_closed", 0xf070},
        {"warning", 0xf071},
        {"chevron_down", 0xf078},
        {"chevron_left", 0xf079},
        {"folder", 0xf07b},
        {"folder_open", 0xf07c},
        {"key", 0xf084},
        {"external_link", 0xf093},
        {"copy", 0xf0c5},
        {"save", 0xf0c7},
        {"caret_down", 0xf0d7},
        {"caret_left", 0xf0da},
        {"bolt", 0xf0e7},
        {"lightbulb", 0xf0eb},
        {"file", 0xf0f6},
        {"folder_plus", 0xf65e},
        {"circle", 0xf111},
        {"cube", 0xf1b2},
        {"code", 0xf121},
        {"play_circle", 0xf144},
        {"bug", 0xf188},
        {"trash", 0xf1f8},
        {"trash_alt", 0xf2ed},
        {"directory", 0xf07b},
        {"directory_open", 0xf07c},
        {"eye", 0xf06e},
        {"gear", 0xf013},
        {"remove", 0xf2ed},
        {"close", 0xf00d},
        {"check", 0xf00c},
        {"plus", 0xf067},
        {"minus", 0xf068}
    };

    const auto found = glyphs.find(name);
    if (found == glyphs.end())
        throw std::invalid_argument(
            "Unknown BGUI Nerd Font icon: " + std::string(name)
        );

    return found->second;
}

std::string encode_utf8(const char32_t codepoint) {
    std::string result;
    if (codepoint <= 0x7f) {
        result.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7ff) {
        result.push_back(static_cast<char>(0xc0 | (codepoint >> 6)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else if (codepoint <= 0xffff) {
        result.push_back(static_cast<char>(0xe0 | (codepoint >> 12)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else {
        result.push_back(static_cast<char>(0xf0 | (codepoint >> 18)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    return result;
}

}

bgui::icon::icon(const std::string_view name, const float scale)
    : text(encode_utf8(find_icon_glyph(name)), scale) {
    type = "icon";
    style.visual.font = "nerd_font";
    set_wrap(false);
}

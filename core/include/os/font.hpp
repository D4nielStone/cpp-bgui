#pragma once
#include <string>
#include <utility>
#include <unordered_map>
#include "utils/vec.hpp"
#include "utils/draw.hpp"

namespace bgui {
    // \brief Represents a single character glyph in a font.
    struct character {
        bgui::vec<2, unsigned int> size{0, 0};
        bgui::vec<2, int> bearing{0, 0};
        long int advance{0};
        bgui::vec2 uv_min{0.f};
        bgui::vec2 uv_max{0.f};
    };

    // \brief Represents a font with its character glyphs and texture atlas.
    struct font {      
        bgui::texture atlas{};
        std::unordered_map<char32_t, bgui::character> chs{};
        unsigned int resolution = 0;
        float ascent = 0.0f;
        float descent = 0.0f;
        float line_gap = 0.0f;
    };

    class font_manager {
    public:
        std::queue<std::string> m_font_queue;
        std::unordered_map<std::string, font> m_fonts;
        static constexpr float m_default_resolution = 40.f;
        static font_manager& get_instance();
        font_manager();
        ~font_manager();
        font& get_font(const std::string& name);
        bool has_font(const std::string& name);
    };
} // namespace bgui
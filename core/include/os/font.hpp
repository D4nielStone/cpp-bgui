#pragma once
#include <cstddef>
#include <queue>
#include <string>
#include <utility>
#include <unordered_map>
#include <functional>
#include "utils/vec.hpp"
#include "utils/draw.hpp"

namespace bgui {
    enum class font_style {
        regular,
        bold,
        italic,
        bold_italic,
        light,
        semibold,
        black,
        thin
    };

    const char* font_style_name(font_style style);

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
        std::string family;
        std::string style;
        unsigned int resolution = 0;
        float ascent = 0.0f;
        float descent = 0.0f;
        float line_gap = 0.0f;
    };

    class font_manager {
    public:
        std::queue<std::string> m_font_queue;
        std::unordered_map<std::string, font> m_fonts;
        std::function<void(const font&)> m_on_font_loaded;
        std::unordered_map<unsigned int, std::string> m_default_font_keys;
        static constexpr float m_default_resolution = 40.f;
        static font_manager& get_instance();
        font_manager();
        ~font_manager() = default;
        font& get_font(const std::string& name, unsigned int resolution = m_default_resolution);
        bool has_font(const std::string& name, unsigned int resolution = m_default_resolution) const;
        static std::string make_key(const std::string& name, unsigned int resolution);
        void set_font_loaded_callback(std::function<void(const font&)> callback);
        void set_default_font(const std::string& name, unsigned int resolution = m_default_resolution);
    };
} // namespace bgui
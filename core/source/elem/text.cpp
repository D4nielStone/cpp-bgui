#include "elem/text.hpp"
#include <bgui.hpp>
#include <iostream>
#include <codecvt>
#include <locale>
#include <vector>

// Converte UTF-8 -> UTF-32 e também devolve, para cada codepoint,
// quantos bytes ele ocupava na string original. Isso é essencial
// para não misturar "índice de codepoint" com "offset em bytes"
// ao usar std::string::substr().
std::u32string utf8_to_utf32(const std::string& str, std::vector<size_t>* byte_lengths = nullptr) {
    std::u32string result;
    size_t i = 0;
    while (i < str.length()) {
        unsigned char c = str[i];
        char32_t ch = 0;
        size_t len = 1;

        if ((c & 0x80) == 0) {
            ch = c;
            len = 1;
        } else if ((c & 0xE0) == 0xC0) {
            ch = ((c & 0x1F) << 6) | (str[i + 1] & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            ch = ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
            len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            ch = ((c & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
            len = 4;
        }

        result.push_back(ch);
        if (byte_lengths) byte_lengths->push_back(len);
        i += len;
    }
    return result;
}

bgui::text::text(const std::string &buffer, float scale) : m_buffer(buffer), m_scale(scale) {
    type = "text";
    set_font(computed_style.visual.font);
    m_material.m_use_tex = true;
    m_material.m_shader_tag = "ui::text";
}
bgui::text::~text() {
}
void bgui::text::on_update() {
    element::on_update();
    // If the style changes, update the font.
    if(m_last_font != computed_style.visual.font) {
        set_font(computed_style.visual.font);
        m_last_font = computed_style.visual.font;
    }
}

void bgui::text::calc_content_size(const layer&) {
    auto& font_manager = bgui::font_manager::get_instance();
    if (!font_manager.has_font(computed_style.visual.font)) {
        set_content_size({0, 0});
        return;
    }
    const auto& font = font_manager.get_font(computed_style.visual.font);
    if (font.chs.empty()) {
        set_content_size({0, 0});
        return;
    }

    const float scale = m_scale * bgui::get_global_scale();
    const float line_height = (font.ascent + font.descent + font.line_gap) * scale;
    const float wrap_width = 500.f * bgui::get_global_scale();
    float line_width = 0.f;
    float max_width = 0.f;
    int line_count = 1;

    for (char32_t character : utf8_to_utf32(m_buffer)) {
        if (character == U'\n') {
            max_width = std::max(max_width, line_width);
            line_width = 0.f;
            ++line_count;
            continue;
        }

        const auto glyph = font.chs.find(character);
        if (glyph == font.chs.end()) {
            continue;
        }

        const float advance = glyph->second.advance * scale;
        if (line_width + advance > wrap_width && line_width > 0.f) {
            max_width = std::max(max_width, line_width);
            line_width = 0.f;
            ++line_count;
        }
        line_width += advance;
    }

    max_width = std::max(max_width, line_width);
    set_content_size({
        static_cast<int>(max_width),
        static_cast<int>(line_count * line_height)
    });
}
void bgui::text::set_font(const std::string &path) {
    font_manager::get_instance().m_font_queue.push(path);
}

float bgui::text::get_text_width(const std::string& t) {
    auto& font_manager = bgui::font_manager::get_instance();
    if (!font_manager.has_font(computed_style.visual.font))
        return 0.0f;
    const auto& chs = font_manager.get_font(computed_style.visual.font).chs;
    if (chs.empty()) return 0.0f;
    const float scale = m_scale * bgui::get_global_scale();

    float line_x = 0.f;
    float max_line_width = 0.0f;

    for (char32_t ca : utf8_to_utf32(t)) {
        if (ca == U'\n') {
            max_line_width = std::max(max_line_width, line_x);
            line_x = 0.f;
            continue;
        }

        auto it = chs.find(ca);
        if (it == chs.end()) continue;
        const auto& ch = it->second;

        line_x += ch.advance * scale;
    }

    // Garante que a última linha (que não termina em '\n') também seja considerada
    max_line_width = std::max(max_line_width, line_x);

    return max_line_width;
}

void bgui::text::get_requires(bgui::draw_data* data) {
    auto& font_manager = bgui::font_manager::get_instance();
    if (!font_manager.has_font(computed_style.visual.font))
        return;
    const auto& font = font_manager.get_font(computed_style.visual.font);
    const auto& chs = font.chs;
    if (chs.empty()) return;

    const float scale = m_scale * bgui::get_global_scale();
    float ascent   = font.ascent * scale;
    float descent  = font.descent * scale;
    float line_gap = font.line_gap * scale;

    float line_y = ascent;
    float line_x = 0.f;
    float max_line_width = 0.0f;
    int line_count = 1;
    int total_width = get_text_width(m_buffer);

    m_material.m_texture = font.atlas;

    // Converte a string inteira UMA única vez, evitando reconverter
    // e re-medir substrings dentro do loop (era O(n^2)) e evitando
    // usar índices de codepoint em std::string::substr (bug de bytes).
    std::vector<size_t> byte_lengths;
    std::u32string codepoints = utf8_to_utf32(m_buffer, &byte_lengths);
    size_t byte_position = 0;
    float cursor_x = 0.f;
    float cursor_y = 0.f;
    bool cursor_position_found = false;

    auto draw_glyph = [&](char32_t character, float x, float y) {
        auto it = chs.find(character);
        if (it == chs.end()) return;
        const auto& ch = it->second;

        int originx = processed_x();
        switch(computed_style.layout.align.x) {
            case bgui::alignment::start:
                originx = processed_x();
                break;
            case bgui::alignment::center:
                originx = processed_x() + (processed_width() - total_width) / 2;
                break;
            case bgui::alignment::end:
                originx = processed_x() + (processed_width() - total_width);
                break;
        }

        float xpos = originx + x + scale * ch.bearing[0];
        float ypos = processed_y() + y - (ch.bearing[1] * scale - ch.size[1] * scale);
        set_properties();
        data->enqueue({
            m_material, 6,
            { xpos, ypos, scale * ch.size[0], -scale * ch.size[1] },
            ch.uv_min, ch.uv_max,
        });
    };

    for (size_t index = 0; index < codepoints.size(); ++index) {
        const char32_t ca = codepoints[index];
        // Mede o glifo atual para decidir, incrementalmente, se ele
        // ainda cabe na linha (equivalente ao antigo "> 100", mas
        // sem refazer a conversão/medida do zero a cada caractere).
        float advance_this_char = 0.f;
        if (ca != U'\n') {
            auto it_measure = chs.find(ca);
            if (it_measure != chs.end()) {
                advance_this_char = it_measure->second.advance * scale;
            }
        }

        if (ca == U'\n') {
            if (m_cursor_visible && byte_position == m_cursor_position) {
                cursor_x = line_x;
                cursor_y = line_y;
                cursor_position_found = true;
            }
            // End of line
            max_line_width = std::max(max_line_width, line_x);
            line_y += (ascent + descent + line_gap);
            line_x = 0.f;
            line_count++;
            byte_position += byte_lengths[index];
            continue;
        }

        if (line_x + advance_this_char > 500.f) {
            max_line_width = std::max(max_line_width, line_x);
            line_y += (ascent + descent + line_gap);
            line_x = 0.f;
            line_count++;
        }
        if (m_cursor_visible && byte_position == m_cursor_position) {
            cursor_x = line_x;
            cursor_y = line_y;
            cursor_position_found = true;
        }

        draw_glyph(ca, line_x, line_y);
        if (auto it = chs.find(ca); it != chs.end()) {
            line_x += it->second.advance * scale;
        }
        byte_position += byte_lengths[index];
    }

    if (m_cursor_visible && byte_position == m_cursor_position) {
        cursor_x = line_x;
        cursor_y = line_y;
        cursor_position_found = true;
    }

    if (cursor_position_found) {
        const float caret_offset = std::max(1.f, 0.35f * scale);
        draw_glyph(U'|', cursor_x + caret_offset, cursor_y);
    }

    max_line_width = std::max(max_line_width, line_x);
    int total_height = line_count * (ascent + descent + line_gap);

    set_content_size({total_width, total_height});
}
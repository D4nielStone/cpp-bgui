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
void bgui::text::set_font(const std::string &path) {
    // 1: update the queue
    font_manager::get_instance().m_font_queue.push(path);
}

float bgui::text::get_text_width(const std::string& t) {
    const auto& chs = bgui::font_manager::get_instance().get_font(computed_style.visual.font).chs;
    if (chs.empty()) return 0.0f;

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

        line_x += ch.advance * m_scale;
    }

    // Garante que a última linha (que não termina em '\n') também seja considerada
    max_line_width = std::max(max_line_width, line_x);

    return max_line_width;
}

void bgui::text::get_requires(bgui::draw_data* data) {
    const auto& font = bgui::font_manager::get_instance().get_font(computed_style.visual.font);
    const auto& chs = font.chs;
    if (chs.empty()) return;

    float ascent   = font.ascent * m_scale;
    float descent  = font.descent * m_scale;
    float line_gap = font.line_gap * m_scale;

    float line_y = ascent;
    float line_x = 0.f;
    float max_line_width = 0.0f;
    int line_count = 1;
    int total_width = get_text_width(m_buffer);

    m_material.m_texture = font.atlas;

    // Converte a string inteira UMA única vez, evitando reconverter
    // e re-medir substrings dentro do loop (era O(n^2)) e evitando
    // usar índices de codepoint em std::string::substr (bug de bytes).
    std::u32string codepoints = utf8_to_utf32(m_buffer);

    for (char32_t ca : codepoints) {
        // Mede o glifo atual para decidir, incrementalmente, se ele
        // ainda cabe na linha (equivalente ao antigo "> 100", mas
        // sem refazer a conversão/medida do zero a cada caractere).
        float advance_this_char = 0.f;
        if (ca != U'\n') {
            auto it_measure = chs.find(ca);
            if (it_measure != chs.end()) {
                advance_this_char = it_measure->second.advance * m_scale;
            }
        }

        bool break_line = (ca == U'\n') || (line_x + advance_this_char > 500.f);
        if (break_line) {
            // End of line
            max_line_width = std::max(max_line_width, line_x);
            line_y += (ascent + descent + line_gap);
            line_x = 0.f;
            line_count++;

            // Se a quebra não foi por '\n', o caractere atual pertence
            // à nova linha e ainda precisa ser desenhado.
            if (ca == U'\n') {
                continue;
            }
        }

        // Get glyph
        auto it = chs.find(ca);
        if (it == chs.end()) continue;
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

        float xpos = originx + line_x + m_scale * ch.bearing[0];
        float ypos = processed_y() + line_y - (ch.bearing[1] * m_scale - ch.size[1] * m_scale);

        float w = m_scale * ch.size[0];
        float h = m_scale * ch.size[1];

        set_properties();
        data->m_quad_requires.push({
            m_material, 6,
            { xpos, ypos, w, -h },
            ch.uv_min, ch.uv_max,
        });

        line_x += ch.advance * m_scale;
    }

    max_line_width = std::max(max_line_width, line_x);
    int total_height = line_count * (ascent + descent + line_gap);

    set_content_size({total_width, total_height});
}
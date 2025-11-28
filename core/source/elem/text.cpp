#include "elem/text.hpp"
#include <bgui.hpp>
#include <iostream>
#include <codecvt>
#include <locale>

std::u32string utf8_to_utf32(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(str);
}

belem::text::text(const std::string &buffer, float scale) : m_buffer(buffer),
    m_font_name("Noto Sans-Condensed"), m_scale(scale) {
    set_font(m_font_name);
    apply_theme(bgui::instance().get_theme());
    m_material.m_use_tex = true;
    m_material.m_shader_tag = "ui::text";
}
belem::text::~text() {
}
void belem::text::update() {
}
void belem::text::set_font(const std::string &path) {
    auto& i = bos::font_manager::instance();
    m_font = i.get_font(path);
}

void belem::text::get_requests(butil::draw_data& data) {
    const auto& chs = m_font.chs;
    if (chs.empty()) return;
    float ascent = m_font.ascent * m_scale;
    float descent = m_font.descent * m_scale;
    float line_gap = m_font.line_gap * m_scale;

    float line_y = ascent;
    float line_x = 0.f;
    float max_line_width = 0.0f;
    int line_count = 1;

    //m_material.m_texture = m_font.atlas; TODO: change to butil::texture

    for (char32_t ca : utf8_to_utf32(m_buffer)) {
        if (ca == U'\n') {
            // End of line
            max_line_width = std::max(max_line_width, line_x);
            line_y += (ascent + descent + line_gap);
            line_x = 0.f;
            line_count++;
            continue;
        }

        // Get glyph
        auto it = chs.find(ca);
        if (it == chs.end()) continue;
        const auto& ch = it->second;

        float xpos = get_x() + line_x + m_scale * ch.bearing[0];
        float ypos = get_y() + line_y - (ch.bearing[1] * m_scale - ch.size[1] * m_scale);

        float w = m_scale * ch.size[0];
        float h = m_scale * ch.size[1];

        data.m_quad_requests.push_back({
            m_material, 6,
            { xpos, ypos, w, -h },
            ch.uv_min, ch.uv_max
        });

        line_x += ch.advance * m_scale;
    }

    // Update width and height after loop
    max_line_width = std::max(max_line_width, line_x);
    float total_height = line_count * (ascent + descent + line_gap);

    set_size(max_line_width, total_height);
}

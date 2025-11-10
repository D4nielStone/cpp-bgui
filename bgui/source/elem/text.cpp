#include "elem/text.hpp"
#include <bgui.hpp>
#include <iostream>

elements::text::text(const std::string &buffer, unsigned int scale) : m_buffer(buffer), m_font_path("assets/roboto.ttf"), m_scale(scale) {
    set_font(m_font_path);
    set_theme(bgui::instance().get_theme());
    bgui::instance().add_gl_call([&](){
        m_material.m_shader.compile("assets/quad.vs", "assets/text.fs");
    });
}
elements::text::~text() {
}
void elements::text::update() {
}
void elements::text::set_font(const std::string &path) {
    static int id = 0;
    id++;
    m_font = bos::font_manager::instance().load_font(m_buffer + ":" + std::to_string(id), path, m_scale);
}
void elements::text::get_draw_calls(std::vector<draw_call>& calls) {
    const auto& chs = m_font.chs;
    float line_y = m_scale;
    float line_x = 0; 
    float init_x = get_x();
        
    m_material.m_texture = m_font.atlas;
        
    int total_width = 0, total_height = 0;
    for(const auto& ca : m_buffer) {
        if (chs.empty()) return;
        // break line
        if(ca == '\n') {line_y += m_scale; line_x = 0; continue;}

        // set char
        bos::character ch{};
        if(chs.find(ca) != chs.end()) ch = chs.at(ca);
        else continue;
        

        float xpos = get_x() + line_x + ch.bearing[0];
        float ypos = get_y() + line_y - (ch.bearing[1] - ch.size[1]);

        float w = ch.size[0];
        float h = ch.size[1];
    
        calls.push_back({
            m_material, bgui::instance().get_quad_vao(), GL_TRIANGLES, 6, {
                xpos, ypos, w, -h
            }, ch.uv_min, ch.uv_max
        });

        line_x += ch.advance;
        total_height = line_y;
    }
    total_width = line_x;
    set_size(total_width, total_height);
};
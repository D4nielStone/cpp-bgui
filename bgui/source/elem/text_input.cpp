#include "elem/text_input.hpp"

elements::text_input::text_input(const std::string& buffer, const float scale, const std::string& buttom_label,
    const std::function<void()>& f, const std::string& placeholder)
    : m_buffer(buffer, scale), m_buttom(buttom_label, f), m_placeholder(placeholder) {
    apply_theme(bgui::instance().get_theme());
    m_material.m_visible = true;
    m_material.m_use_tex = true;
    m_material.m_shader.compile("quad.vs", "text.fs");
}

void elements::text_input::on_clicked() {
}

void elements::text_input::on_released() {
}

void elements::text_input::on_mouse_hover() {
}

void elements::text_input::update() {
    
}

void elements::text_input::get_draw_requests(std::vector<butil::draw_request>& calls) {
    m_buffer.get_draw_requests(calls);
    m_buttom.get_draw_requests(calls);
}
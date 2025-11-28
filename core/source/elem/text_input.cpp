#include "elem/text_input.hpp"
#include "bgui.hpp"

belem::text_input::text_input(const std::string& buffer, const float scale, const std::string& placeholder) :
    m_buffer(buffer, scale), m_placeholder(placeholder) {
    apply_theme(bgui::instance().get_theme());
    m_visible = false;
    m_material.m_shader_tag = "ui::default";
}

belem::text_input::~text_input()
{
}

void belem::text_input::on_clicked()
{
}

void belem::text_input::on_released() {
}

void belem::text_input::on_mouse_hover() {
}

void belem::text_input::update() {
    
}

void belem::text_input::get_requests(butil::draw_data& data) {
    element::get_requests(data);
    m_buffer.get_requests(data);
}
void belem::text_input::apply_theme(const butil::theme& t) {
    m_buffer.apply_theme(t);
}
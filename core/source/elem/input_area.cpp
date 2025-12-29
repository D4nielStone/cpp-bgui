#include "elem/input_area.hpp"
#include "os/os.hpp"
#include "bgui.hpp"

bgui::input_area::input_area(const std::string& buffer, const float scale, const std::string& placeholder) :
    linear(), m_placeholder(placeholder) {
    m_visible = true;
    recives_input(true);
    set_margin(10, 2);
    m_material.m_shader_tag = "ui::default";
    m_text = &add<text>(buffer, scale);
    m_text->set_margin(5, 0);
    m_text->recives_input(false);
    m_text->request_width(mode::stretch);
    request_width(mode::wrap_content);
    request_height(mode::wrap_content);
}

bgui::input_area::~input_area() {
}

void bgui::input_area::on_pressed() {
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_clicked() {
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_released() {
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_mouse_hover() {
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

bgui::text& bgui::input_area::get_label() {
    return *m_text;
}

void bgui::input_area::update() {
    if(m_input_buffer.empty()) {
        m_text->set_buffer(m_placeholder);
        m_text->get_material().set("text_color", color{0.5, 0.5, 0.5, 1});
    }
    else {
        m_text->set_buffer(m_input_buffer);
        m_text->get_material().set("text_color", m_style.m_text_color);
    }
    linear::update();
}

void bgui::input_area::get_requests(bgui::draw_data* data) {
    linear::get_requests(data);
}
void bgui::input_area::apply_style(const bgui::style& t) {
    linear::apply_style(t);
}
#include "elem/box.hpp"
#include "bgui.hpp"
    
using namespace elements;

box::box() {
    set_size(50, 50);
    set_theme(bgui::instance().get_theme());
    bgui::instance().add_gl_call([&](){
        m_material.m_shader.compile("quad.vs", "quad.fs");
    });
}

void box::update() {
}
void box::set_theme(const butil::theme &t) {
    m_material.set("u_bg_color", t.m_box_color);
    m_material.set("u_bordered", true);
    m_material.set("u_border_radius", 5.f);
    m_material.set("u_border_size", 5.f);
    m_material.set("u_border_color", t.m_box_color);
    m_material.m_visible = true;
}
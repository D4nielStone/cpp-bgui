#include "elem/element.hpp"
#include "bgui.hpp"

element::element() : m_bounds({0.f, 0.f, 50.f, 50.f}) {
    set_theme(bgui::instance().get_theme());
    m_material.m_visible = false;
}

void element::set_position(int x, int y)
{
    m_bounds[0] = x;
    m_bounds[1] = y;
}

void element::set_size(int width, int height) {
    m_bounds[2] = width;
    m_bounds[3] = height;
}

void element::set_x(int x) {    
    m_bounds[0] = x;
}

void element::set_y(int y) {    
    m_bounds[1] = y;
}

void element::set_rect(int x, int y, int width, int height) {
    m_bounds[0] = x;
    m_bounds[1] = y;
    m_bounds[2] = width;
    m_bounds[3] = height;
}

void element::set_shader(const bgl::shader & shd) {
    m_material.m_shader = shd;
}

void element::set_material(const butil::material &mhd) {
    m_material = mhd;
}

int element::get_x() const {
    return m_bounds[0];
}

int element::get_y() const {
    return m_bounds[1];
}
 
bgl::shader& element::get_shader() {
    return m_material.m_shader;
}
 
butil::material & element::get_material() {
    return m_material;
}

int element::get_height() const {
   return m_bounds[3];
}

int element::get_width() const {
    return m_bounds[2];
}


void element::get_draw_calls(std::vector<draw_call>& calls) {
    // by default draw just the background
    calls.push_back({m_material, bgui::instance().get_quad_vao(), GL_TRIANGLES, 6,
         m_bounds});
};
    
void element::update() {
}
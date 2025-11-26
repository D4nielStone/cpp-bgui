#include "elem/element.hpp"
#include "bgui.hpp"

using namespace butil;

element::element() : m_orientation(orientation::horizontal), m_alignment(alignment::start), m_spacing_elements(1) {
    apply_theme(bgui::instance().get_theme());
    m_material.m_visible = false;
}

void element::set_extern_spacing(int x, int y) {
    m_extern_spacing[0] = x;
    m_extern_spacing[1] = y;
}

void element::set_padding(int x, int y) {
    m_padding[0] = x;
    m_padding[1] = y;
}
void element::set_intern_spacing(int x, int y) {
    m_intern_spacing[0] = x;
    m_intern_spacing[1] = y;
}
void element::set_position(int x, int y) {
    m_bounds[0] = x;
    m_bounds[1] = y;
}

void element::set_size(int width, int height) {
    m_bounds[2] = width;
    m_bounds[3] = height;
}

void element::set_height(int h)
{
    m_bounds[3] = h;
}
void element::set_width(int w)
{
    m_bounds[2] = w;
}
void element::set_x(int x)
{
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

void element::set_cross_aligniment(const butil::alignment &al) {
    m_cross_alignment = al;
}
void element::set_aligniment(const butil::alignment &al)
{
    m_alignment = al;
}
void element::set_orientation(const butil::orientation &o) {
    m_orientation = o;
}
void element::set_spacing_elements(const unsigned int a, const unsigned int b) {
    m_spacing_elements = {a, b};
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

butil::vec2i element::get_size() const {
    return vec2i{(int)m_bounds[2], (int)m_bounds[3]};
}
butil::vec2i element::get_position() const {
    return vec2i{(int)m_bounds[0], (int)m_bounds[1]};
}
butil::vec<2, unsigned int> element::get_extern_spacing() const
{
    return m_extern_spacing;
}
int element::get_width() const
{
    return m_bounds[2];
}

void element::get_draw_requests(std::vector<butil::draw_request>& calls) {
    static bool shader_compiled = false;
    if(!shader_compiled) {
        m_material.m_shader.compile("quad.vs", "quad.fs");
        shader_compiled = true;
    }
    // by default draw just the background
    calls.push_back({m_material, bgui::instance().get_quad_vao(), GL_TRIANGLES, 6,
         m_bounds});
};
    
void element::update() {
}
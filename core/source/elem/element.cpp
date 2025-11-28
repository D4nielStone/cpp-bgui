#include "elem/element.hpp"
#include "bgui.hpp"

using namespace butil;

element::element() : m_orientation(orientation::horizontal), m_alignment(alignment::start), m_spacing_elements(1) {
    apply_theme(bgui::instance().get_theme());
    m_visible = false;
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

void element::set_height(float h, butil::mode m)
{
    m_size_mode[1] = m;
    m_bounds[3] = h;
}
void element::set_width(float w, butil::mode m)
{
    m_size_mode[0] = m;
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

void element::set_shader_tag(const std::string &shd) {
    m_material.m_shader_tag = shd;
}

void element::set_material(const butil::material &mhd) {
    m_material = mhd;
}

void element::set_cross_alignment(const butil::alignment &al) {
    m_cross_alignment = al;
}
void element::set_alignment(const butil::alignment &al)
{
    m_alignment = al;
}
void element::set_orientation(const butil::orientation &o) {
    m_orientation = o;
}
void element::set_spacing_elements(const unsigned int a, const unsigned int b) {
    m_spacing_elements = {a, b};
}
void element::set_visible(bool visible)
{
    m_visible = visible;
}
int element::get_x() const {

    return m_bounds[0];
}

int element::get_y() const {
    return m_bounds[1];
}
 
std::string element::get_shader_tag() const {
    return m_material.m_shader_tag;
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

void element::get_requests(butil::draw_data& data) {
    if(!m_visible) return;
    // by default draw just the background
    data.m_quad_requests.push_back({m_material, 6, m_bounds});
};
    
void element::update() {
}
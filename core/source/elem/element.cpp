#include "elem/element.hpp"
#include "bgui.hpp"

using namespace bgui;

element::element() : m_orientation(orientation::horizontal), m_alignment(alignment::start), m_spacing_elements(1) {
    apply_theme(bgui::get_theme());
    m_visible = false;
}

void element::set_intern_spacing(int x, int y) {
    m_intern_spacing[0] = x;
    m_intern_spacing[1] = y;
    m_intern_spacing[2] = x;
    m_intern_spacing[3] = y;
}

void element::set_margin(int left, int top, int right, int bottom) {
    m_margin[0] = left;
    m_margin[1] = top;
    m_margin[2] = right;
    m_margin[3] = bottom;
}

void element::set_margin(int x, int y) {
    set_margin(x, y, x, y);
}

void element::set_padding(int left, int top, int right, int bottom) {
    m_padding[0] = left;
    m_padding[1] = top;
    m_padding[2] = right;
    m_padding[3] = bottom;
}

void element::set_padding(int x, int y) {
    set_padding(x, y, x, y);
}

void element::set_min_width(float w) { m_min_width = w; }
void element::set_max_width(float w) { m_max_width = w; }
void element::set_min_height(float h) { m_min_height = h; }
void element::set_max_height(float h) { m_max_height = h; }
void element::set_position(int x, int y) {
    m_bounds[0] = x;
    m_bounds[1] = y;
}

void element::set_size(int width, int height) {
    m_bounds[2] = width;
    m_bounds[3] = height;
}

void element::set_height(float h, bgui::mode m)
{
    m_size_mode[1] = m;
    m_bounds[3] = h;
}
void element::set_width(float w, bgui::mode m)
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

void element::set_material(const bgui::material &mhd) {
    m_material = mhd;
}

void element::set_cross_alignment(const bgui::alignment &al) {
    m_cross_alignment = al;
}
void element::set_alignment(const bgui::alignment &al)
{
    m_alignment = al;
}
void element::set_orientation(const bgui::orientation &o) {
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
 
bgui::material & element::get_material() {
    return m_material;
}

int element::get_height() const {
   float h = m_bounds[3];
   switch (m_size_mode[1]) {
       case bgui::mode::pixel:
           break;
       case bgui::mode::relative:
           if (m_parent) h = (float)m_parent->get_size()[1] * m_bounds[3];
           break;
       case bgui::mode::wrap_content:
           if (as_layout()) {
               const_cast<bgui::layout*>(as_layout())->fit_to_content();
               h = m_bounds[3];
           }
           break;
       case bgui::mode::match_parent:
           if (m_parent) {
               int parent_h = m_parent->get_size()[1];
               h = parent_h - (m_margin[1] + m_margin[3]) - (m_parent->m_padding[1] + m_parent->m_padding[3]);
           }
           break;
   }
   if (m_min_height > 0.f && h < m_min_height) h = m_min_height;
   if (m_max_height > 0.f && h > m_max_height) h = m_max_height;
   return (int)h;
}

bgui::vec2i element::get_size() const {
    int w = (int)m_bounds[2];
    int h = (int)m_bounds[3];
    // width
    switch (m_size_mode[0]) {
        case bgui::mode::pixel:
            break;
        case bgui::mode::relative:
            if (m_parent) w = (int)((float)m_parent->get_size()[0] * m_bounds[2]);
            break;
        case bgui::mode::wrap_content:
            if (as_layout()) {
                const_cast<bgui::layout*>(as_layout())->fit_to_content();
                w = (int)m_bounds[2];
            }
            break;
        case bgui::mode::match_parent:
            if (m_parent) {
                int parent_w = m_parent->get_size()[0];
                w = parent_w - (int)(m_margin[0] + m_margin[2]) - (int)(m_parent->m_padding[0] + m_parent->m_padding[2]);
            }
            break;
    }
    if (m_min_width > 0.f && w < (int)m_min_width) w = (int)m_min_width;
    if (m_max_width > 0.f && w > (int)m_max_width) w = (int)m_max_width;
    return vec2i{w, h};
}
bgui::vec2i element::get_position() const {
    return vec2i{(int)m_bounds[0], (int)m_bounds[1]};
}
bgui::vec<4, unsigned int> element::get_margin() const
{
    return m_margin;
}
int element::get_width() const
{
    return get_size()[0];
}

void element::get_requests(bgui::draw_data* data) {
    if(!m_visible) return;
    // by default draw just the background
    data->m_quad_requests.push({m_material, 6, m_bounds});
};
    
void element::update() {
}
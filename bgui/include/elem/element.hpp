// \brief The element is the base class for all GUI parts.

#pragma once
#include <vector>
#include <memory>
#include "utils/theme.hpp"

class layout;
class element {
protected:
    butil::orientation m_orientation = butil::orientation::horizontal;
    butil::alignment m_alignment = butil::alignment::start, m_cross_alignment = butil::alignment::start;
    butil::vec<2, unsigned int> m_spacing_elements;
    butil::material m_material;

    // absolute position and size
    butil::vec<2, unsigned int> m_intern_spacing {0, 0}, m_extern_spacing{0, 0}, m_padding{0, 0};
    butil::vec4 m_bounds{0, 0, 20, 20};
public:
    element();
    virtual ~element() = default;
    // \brief setters
    void set_intern_spacing(int x, int y);
    void set_extern_spacing(int x, int y);
    void set_padding(int x, int y);
    void set_position(int x, int y);
    void set_size(int width, int height);
    void set_height(int h);
    void set_width(int h);
    void set_x(int x);
    void set_y(int y);
    void set_rect(int x, int y, int width, int height);
    void set_shader(const bgl::shader& shd);
    void set_material(const butil::material& mhd);
    void set_cross_aligniment(const butil::alignment& al);
    void set_aligniment(const butil::alignment& al);
    void set_orientation(const butil::orientation& o);
    void set_spacing_elements(const unsigned int a, const unsigned int b);
    // \brief getters
    int get_x() const;
    int get_y() const;
    int get_width() const;
    int get_height() const;
    butil::vec2i get_size() const;
    butil::vec2i get_position() const;
    butil::vec<2, unsigned int> get_extern_spacing() const;
    bgl::shader& get_shader();
    butil::material& get_material();

    // \brief virtual functions
    virtual layout* as_layout() { return nullptr; }
    virtual void update();
    virtual void apply_theme(const butil::theme& theme){};
    virtual void on_clicked() {};
    virtual void on_released() {};
    virtual void on_mouse_hover() {};
    virtual void get_draw_requests(std::vector<butil::draw_request>& calls);
};
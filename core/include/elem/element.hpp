// \brief The element is the base class for all GUI parts.

#pragma once
#include <vector>
#include <memory>
#include "utils/theme.hpp"
#include "utils/draw.hpp"

namespace blay{class layout;}
class element {
protected:
    butil::orientation m_orientation = butil::orientation::horizontal;
    butil::alignment m_alignment = butil::alignment::start, m_cross_alignment = butil::alignment::start;
    butil::vec<2, unsigned int> m_spacing_elements{0, 0};
    butil::material m_material;
    bool m_visible {true};

    // absolute position and size
    butil::vec<2, butil::mode> m_size_mode {butil::mode::pixel, butil::mode::pixel};
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
    void set_height(float h, butil::mode m = butil::mode::pixel);
    void set_width(float h, butil::mode m = butil::mode::pixel);
    void set_x(int x);
    void set_y(int y);
    void set_rect(int x, int y, int width, int height);
    void set_shader_tag(const std::string &shd);
    void set_material(const butil::material& mhd);
    void set_cross_alignment(const butil::alignment& al);
    void set_alignment(const butil::alignment& al);
    void set_orientation(const butil::orientation& o);
    void set_spacing_elements(const unsigned int a, const unsigned int b);
    void set_visible(bool);
    // \brief getters
    int get_x() const;
    int get_y() const;
    int get_width() const;
    int get_height() const;
    butil::vec2i get_size() const;
    butil::vec2i get_position() const;
    butil::vec<2, unsigned int> get_extern_spacing() const;
    std::string get_shader_tag() const;
    butil::material& get_material();

    // \brief virtual functions
    virtual blay::layout* as_layout() { return nullptr; }
    virtual void update();
    virtual void apply_theme(const butil::theme& theme){};
    virtual void on_clicked() {};
    virtual void on_released() {};
    virtual void on_mouse_hover() {};
    virtual void get_requests(butil::draw_data& calls);
};
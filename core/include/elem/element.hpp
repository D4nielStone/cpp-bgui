/**
    * @class Element
    * @file element.hpp
    */

#pragma once
#include <vector>
#include <memory>
#include "utils/theme.hpp"
#include "utils/draw.hpp"

namespace bgui{class layout;}
class element {
protected:
    bgui::orientation m_orientation = bgui::orientation::horizontal;
    bgui::alignment m_alignment = bgui::alignment::start, m_cross_alignment = bgui::alignment::start;
    bgui::vec<2, unsigned int> m_spacing_elements {0, 0};
    element* m_parent {nullptr};
    bgui::material m_material;
    bool m_visible {true};

    // absolute position and size
    bgui::vec<2, bgui::mode> m_size_mode {bgui::mode::pixel, bgui::mode::pixel};
    // per-side: left, top, right, bottom
    bgui::vec<4, unsigned int> m_intern_spacing {0,0,0,0};
    bgui::vec<4, unsigned int> m_margin {0,0,0,0};
    bgui::vec<4, unsigned int> m_padding {0,0,0,0};
    // min/max constraints (0 means no constraint)
    float m_min_width{0.f}, m_max_width{0.f}, m_min_height{0.f}, m_max_height{0.f};
    bgui::vec4 m_bounds{0, 0, 0, 0};
public:
    element();
    virtual ~element() = default;
    // \brief setters
    void set_intern_spacing(int x, int y);
    // per-side margin: left, top, right, bottom
    void set_margin(int left, int top, int right, int bottom);
    // convenience: symmetric margin
    void set_margin(int x, int y);
    // per-side padding
    void set_padding(int left, int top, int right, int bottom);
    void set_padding(int x, int y);
    // min/max constraints
    void set_min_width(float w);
    void set_max_width(float w);
    void set_min_height(float h);
    void set_max_height(float h);
    void set_position(int x, int y);
    void set_size(int width, int height);
    void set_height(float h, bgui::mode m = bgui::mode::pixel);
    void set_width(float h, bgui::mode m = bgui::mode::pixel);
    void set_x(int x);
    void set_y(int y);
    void set_rect(int x, int y, int width, int height);
    void set_shader_tag(const std::string &shd);
    void set_material(const bgui::material& mhd);
    void set_cross_alignment(const bgui::alignment& al);
    void set_alignment(const bgui::alignment& al);
    void set_orientation(const bgui::orientation& o);
    void set_spacing_elements(const unsigned int a, const unsigned int b);
    void set_visible(bool);
    void set_parent(element* parent) { m_parent = parent; }
    // \brief getters
    int get_x() const;
    int get_y() const;
    int get_width() const;
    int get_height() const;
    bgui::vec2i get_size() const;
    bgui::vec2i get_position() const;
    // returns margin as {left, top, right, bottom}
    bgui::vec<4, unsigned int> get_margin() const;
    std::string get_shader_tag() const;
    bgui::material& get_material();
    element* get_parent() const { return m_parent; }

    // \brief virtual functions
    virtual bgui::layout* as_layout() { return nullptr; }
    virtual void update();
    virtual void apply_theme(const bgui::theme& theme){};
    virtual void on_clicked() {};
    virtual void on_released() {};
    virtual void on_mouse_hover() {};
    virtual void get_requests(bgui::draw_data* calls);
};
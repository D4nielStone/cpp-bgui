// \brief The element is the base class for all GUI parts.

#pragma once
#include <vector>
#include <memory>
#include "utils/material.hpp"
#include "utils/theme.hpp"

// \brief A draw call structure containing the necessary information to render an element.
struct draw_call {
    butil::material& m_material;
    GLuint m_vao{0};
    GLenum m_mode{GL_TRIANGLES};
    int m_count{6};
    butil::vec4 m_bounds{0.0f, 0.0f, 100.0f, 100.0f};
    butil::vec2 m_uv_min{0, 0};
    butil::vec2 m_uv_max{0, 0};

    bool operator==(const draw_call& other) const {
        return m_material == other.m_material &&
               m_vao == other.m_vao &&
               m_mode == other.m_mode &&
               m_count == other.m_count;
    }
};

class element {
protected:
    butil::material m_material;
    // absolute position and size
    butil::vec4 m_bounds;
public:
    butil::vec<2, unsigned int> m_intern_spacing {0, 0}, m_extern_spacing{0, 0};
    element();
    virtual ~element() = default;
    // \brief setters
    void set_intern_spacing(int x, int y);
    void set_extern_spacing(int x, int y);
    void set_position(int x, int y);
    void set_size(int width, int height);
    void set_x(int x);
    void set_y(int y);
    void set_rect(int x, int y, int width, int height);
    void set_shader(const bgl::shader& shd);
    void set_material(const butil::material& mhd);
    virtual void set_theme(const butil::theme& theme) {
    };
    // \brief getters
    int get_x() const;
    int get_y() const;
    int get_width() const;
    int get_height() const;
    bgl::shader& get_shader();
    butil::material& get_material();

    virtual void update();
    virtual void get_draw_calls(std::vector<draw_call>& calls);
};
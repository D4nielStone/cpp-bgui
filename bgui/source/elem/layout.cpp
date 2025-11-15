#include "elem/layout.hpp"
#include "bgui.hpp"

layout::layout() : m_orientation(orientation::horizontal), m_alignment(alignment::start), m_spacing_elements(1) {
        m_material.m_visible = false;
        
        set_theme(bgui::instance().get_theme());
        bgui::instance().add_gl_call([&](){
            m_material.m_shader.compile("quad.vs", "quad.fs");
        });
    };
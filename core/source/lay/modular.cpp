
#include "lay/modular.hpp"
#include "bgui.hpp"
using namespace bgui;

using namespace bgui;
    
modular::modular(const orientation& ori) {
    m_orientation = ori;
    m_visible = false;
    m_material.m_shader_tag = "ui::default";
}

void modular::update() {
}

void modular::fit_to_content() {
}

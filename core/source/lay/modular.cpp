
#include "lay/modular.hpp"
#include "bgui.hpp"
using namespace bgui;
    
modular::modular(const orientation& ori) : m_orientation(ori) {
    m_visible = false;
}

void modular::on_update() {
}
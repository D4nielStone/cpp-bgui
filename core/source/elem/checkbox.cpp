#include "elem/checkbox.hpp"

void bgui::checkbox::apply_theme(const bgui::theme & t) {
    element::apply_theme(t);
    set_margin(5, 5);
    request_size(14, 14);
    m_material.set("bg_color", bgui::color{0, 0, 0, 0});
}
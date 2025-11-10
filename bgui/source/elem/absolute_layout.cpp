#include "elem/absolute_layout.hpp"
#include "bgui.hpp"

void absolute_layout::get_draw_calls(std::vector<draw_call>& calls) {
    element::get_draw_calls(calls);
    // absolute layouts get the draw call in addition order
    for (auto& elem : m_elements) {
        elem->get_draw_calls(calls);
    }
};
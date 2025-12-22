#include "bgui.hpp"
#include "os/os.hpp"
#include <utils/vec.hpp>
#include <stdexcept>
#include <iostream>

static bool init_trigger = false;
std::unique_ptr<bgui::layout> bgui::m_main_layout;
static std::unique_ptr<bgui::draw_data> m_draw_data;
bgui::style m_style;
static std::queue<std::function<void()>> s_functions;

bgui::layout& bgui::get_layout() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    return *bgui::m_main_layout;
}

void bgui::set_up() {
    init_trigger = true;
    if(!m_main_layout)
        set_layout<bgui::layout>();
    if(!m_draw_data)
        m_draw_data = std::make_unique<bgui::draw_data>();
    // apply default style
    m_style = bgui::dark_style;
    m_main_layout->apply_style(m_style);
}

void bgui::apply_style(const bgui::style& gui_style) {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    // set the style and update params recursively accordingly
    m_style = gui_style;

    m_main_layout->apply_style(m_style);
}

bgui::style& bgui::get_style() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    return m_style;
}
bgui::draw_data* bgui::get_draw_data() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    return m_draw_data.get();
}
bool bgui::shutdown_lib() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    init_trigger = false;
    bgui::m_main_layout.reset();
    m_draw_data.reset();
    return true;
}

bool update_inputs(bgui::layout &lay){
    // global element for last capture
    static bgui::element* g_mouse_captured = nullptr;

    auto m = bgui::get_mouse_position();
    float mx = m[0];
    float my = m[1];

    bool mouse_now = bgui::get_pressed(bgui::input_key::mouse_left);
    bool mouse_click = (mouse_now && !bgui::get_context().m_last_mouse_left);
    bool mouse_released = (!mouse_now && bgui::get_context().m_last_mouse_left);
    
    const auto& elements = lay.get_elements();   

    for (size_t i = elements.size(); i-- > 0; ) {
        auto elem = elements[i].get();

        if (auto* cast = elem->as_layout())
            if(update_inputs(*cast)) {
                return true;
            }
        
        // update the last input-captured element
        if (g_mouse_captured) {
            if (mouse_now) {
                g_mouse_captured->on_pressed();
                g_mouse_captured->on_drag(m - bgui::get_context().m_last_mouse_pos);
            }
            if (mouse_released) {
                g_mouse_captured->on_released();
                g_mouse_captured = nullptr; // release the capture
            }
            return true;
        }

        // inside test
        float x = elem->processed_x();
        float y = elem->processed_y();
        float w = elem->processed_width();
        float h = elem->processed_height();

        bool inside =
            mx >= x &&
            mx <= x + w &&
            my >= y &&
            my <= y + h;

        if (inside) {
            if(!elem->recives_input()) return false;
            elem->on_mouse_hover();
            if (mouse_click) {
                g_mouse_captured = elem; // start capture
                elem->on_clicked();
                elem->on_pressed();
            }
            if(mouse_now) {
                elem->on_pressed();
                elem->on_drag(m - bgui::get_context().m_last_mouse_pos);
            }
            if(mouse_released) {
                elem->on_released();
                if (g_mouse_captured == elem)
                    g_mouse_captured = nullptr; // release capture when mouse release
            }
            return true;
        }
    }
    return false;
}
// Updates the main layout
void bgui::update() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    bgui::vec2i w_size = bgui::get_context_size();
    // the main layout must to be resized based on the window size by default.
    bgui::m_main_layout->request_height(bgui::mode::match_parent);
    bgui::m_main_layout->request_width(bgui::mode::match_parent);
    
    while(!s_functions.empty()) {
        auto& f = s_functions.front();
        f();
        s_functions.pop();
    }

    // update main layout and inputs
    bgui::m_main_layout->update_size(w_size);
    bgui::m_main_layout->update();

    update_inputs(*bgui::m_main_layout);

    bgui::get_context().m_last_mouse_left = bgui::get_pressed(bgui::input_key::mouse_left);
    bgui::get_context().m_last_mouse_pos = bgui::get_mouse_position();

    // get new requests
    if(!get_draw_data()->m_quad_requests.empty()) std::cout << "[BGUI] Warning: draw data not empty at beginning of frame.\nMake sure you are resetting draw data each frame.\n";
    bgui::m_main_layout->get_requests(get_draw_data());
}

void bgui::add_function(const std::function<void()>& f) {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    s_functions.push(f);
}
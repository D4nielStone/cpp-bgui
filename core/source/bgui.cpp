#include "bgui.hpp"
#include "os/os.hpp"
#include "os/style_manager.hpp"
#include <utils/vec.hpp>
#include <stdexcept>
#include <iostream>

static bool init_trigger = false;
std::unique_ptr<bgui::layout> bgui::s_main_layout;
static std::unique_ptr<bgui::draw_data> s_draw_data;
static std::queue<std::function<void()>> s_functions;

bgui::layout& bgui::get_layout() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    return *s_main_layout;
}

void bgui::set_up() {
    //std::cout << "[BGUI] Setting-up the library.\n";
    init_trigger = true;
    // Create the default layout
    if(!s_main_layout)
        set_layout<bgui::layout>();
    // Create draw data structure
    if(!s_draw_data)
        s_draw_data = std::make_unique<bgui::draw_data>();
    // Set up default theme
    auto& sm = style_manager::get_instance();
}

void bgui::cascade_style() {
    if(!init_trigger)
        throw std::runtime_error("[BGUI] You must initialize the library.");

    auto& sm = style_manager::get_instance();

    bgui::s_main_layout->style.layout.require_height(bgui::mode::match_parent);
    bgui::s_main_layout->style.layout.require_width(bgui::mode::match_parent);
    s_main_layout->cascade_style();
}

bgui::draw_data* bgui::get_draw_data() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    return s_draw_data.get();
}
bool bgui::shutdown_lib() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    init_trigger = false;
    bgui::s_main_layout.reset();
    s_draw_data.reset();
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
void bgui::on_update() {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");

    bgui::vec2i w_size = bgui::get_context_size();

    // the main layout must to be resized based on the window size by default.
    while(!s_functions.empty()) {
        auto& f = s_functions.front();
        f();
        s_functions.pop();
    }

    // cascade style
    cascade_style();

    // update main layout and inputs
    get_context().m_actual_cursor = cursor::arrow;
    bgui::s_main_layout->process_required_size(w_size);
    bgui::s_main_layout->on_update();

    // reset cursor
    update_inputs(*bgui::s_main_layout);

    bgui::get_context().m_last_mouse_left = bgui::get_pressed(bgui::input_key::mouse_left);
    bgui::get_context().m_last_mouse_pos = bgui::get_mouse_position();

    // get new requires
    if(!get_draw_data()->m_quad_requires.empty()) std::cout << "[BGUI] Warning: draw data not empty at beginning of frame.\nMake sure you are resetting draw data each frame.\n";
    bgui::s_main_layout->get_requires(get_draw_data());
}

void bgui::add_function(const std::function<void()>& f) {
    if(!init_trigger) throw std::runtime_error("[BGUI] You must initialize the library.");
    s_functions.push(f);
}

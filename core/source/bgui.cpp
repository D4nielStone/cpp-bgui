#include "bgui.hpp"
#include "os/os.hpp"
#include <utils/vec.hpp>
#include <stdexcept>
#include <iostream>

static bool init_trigger = false;
std::unique_ptr<bgui::layout> bgui::m_main_layout;
static std::unique_ptr<bgui::draw_data> m_draw_data;
bgui::theme m_theme;
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
    // apply default theme
    m_theme = bgui::dark_theme;
    m_main_layout->apply_theme(m_theme);
}

void bgui::apply_theme(const bgui::theme& gui_theme) {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    // set the theme and update params recursively accordingly
    m_theme = gui_theme;

    m_main_layout->apply_theme(m_theme);
}

bgui::theme& bgui::get_theme() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    return m_theme;
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
    auto m = bgui::get_mouse_position();
    float mx = m[0];
    float my = m[1];

    bool mouse_now = bgui::get_pressed(bgui::input_key::mouse_left);
    bool mouse_click = (mouse_now && !bgui::get_window().m_last_mouse_left);
    bool mouse_released = (!mouse_now && bgui::get_window().m_last_mouse_left);
    
    const auto& elements = lay.get_elements();   

    for (size_t i = elements.size(); i-- > 0; ) {
        auto elem = elements[i].get();

        if (auto* cast = elem->as_layout())
            if(update_inputs(*cast)) {
                return true;
            }
        
        float x = elem->get_x();
        float y = elem->get_y();
        float w = elem->get_width();
        float h = elem->get_height();

        bool inside =
            mx >= x &&
            mx <= x + w &&
            my >= y &&
            my <= y + h;
    }
    return false;
}
// Updates the main layout
void bgui::update() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    // the main layout must to be resized based on the window size.
    bgui::vec2i w_size = bgui::get_window_size();
    bgui::m_main_layout->set_rect(0.f, 0.f, static_cast<float>(w_size[0]), static_cast<float>(w_size[1]));
    
    while(!s_functions.empty()) {
        auto& f = s_functions.front();
        f();
        s_functions.pop();
    }

    // update main layout and inputs
    bgui::m_main_layout->update();
    update_inputs(*bgui::m_main_layout);
    bgui::get_window().m_last_mouse_left = bgui::get_pressed(bgui::input_key::mouse_left);

    // clean draw data and get new requests
    if(!get_draw_data()->m_quad_requests.empty()) std::cout << "[BGUI] Warning: draw data not empty at beginning of frame.\nMake sure you are resetting draw data each frame.\n";
    bgui::m_main_layout->get_requests(get_draw_data());
}

void bgui::add_function(const std::function<void()>& f) {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    s_functions.push(f);
}
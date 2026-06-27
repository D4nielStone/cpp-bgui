#include "elem/input_area.hpp"
#include "os/os.hpp"
#include "bgui.hpp"

bgui::input_area::input_area(const std::string& buffer, const float scale, std::function<void(const std::string)> action, const std::string& placeholder) :
    linear(), m_placeholder(placeholder), m_input_buffer(buffer), m_enter_func(action) {
    type = "inputarea";
    recives_input(true);
    m_text = &add<text>(m_input_buffer.empty() ? m_placeholder : m_input_buffer, scale);
    m_text->add_class("inputarea-txt");
    m_text->recives_input(false);
}

bgui::input_area::~input_area() {
}

void bgui::input_area::on_pressed() {
    element::on_pressed();
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_clicked() {
    element::on_clicked();
    set_style_state(state::focused);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_released() {
    element::on_released();
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_mouse_hover() {
    element::on_mouse_hover();
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

bgui::text& bgui::input_area::get_label() {
    return *m_text;
}

void bgui::input_area::on_update() {
        auto& ctx = bgui::get_context();
        if(!ctx.m_char_buffer.empty()) {
            m_input_buffer += ctx.m_char_buffer;
            ctx.m_char_buffer.clear();
        }

        auto action = ctx.m_input_map[bgui::input_key::backspace];
        if(action == bgui::input_action::press || action == bgui::input_action::repeat) {
            if(!m_input_buffer.empty()) {
                m_input_buffer.pop_back();
            }
            ctx.m_input_map[bgui::input_key::backspace] = bgui::input_action::none;
        }

    if(m_input_buffer.empty()) {
        m_text->set_buffer(m_placeholder);
        m_text->computed_style.visual.text.a = 0.4f; // make placeholder text semi-transparent
    }
    else {
        m_text->set_buffer(m_input_buffer);
        m_text->computed_style.visual.text.a = 1.f;
    }
    // on enter func
    if(bgui::get_pressed(bgui::input_key::enter)) {
        bgui::add_function([&](){
            m_enter_func(get_buffer());
            set_buffer("");
        });
    }

    linear::on_update();
}

void bgui::input_area::get_requires(bgui::draw_data* data) {
    linear::get_requires(data);
}
#include "elem/inputbox.hpp"
#include "os/os.hpp"
#include "bgui.hpp"
#include <cmath>

namespace {        
    std::string remove_trailing_zeros(std::string s) {
        // Remove trailing zeros after the decimal point
        if (s.find('.') != std::string::npos) {
            s.erase(s.find_last_not_of('0') + 1);

            // Remove the decimal point if nothing follows it
            if (!s.empty() && s.back() == '.')
                s.pop_back();
        }

        return s;
    }

    size_t previous_utf8_character(const std::string& value, size_t position) {
        if (position == 0) return 0;
        --position;
        while (position > 0 &&
               (static_cast<unsigned char>(value[position]) & 0xC0) == 0x80) {
            --position;
        }
        return position;
    }

    size_t next_utf8_character(const std::string& value, size_t position) {
        if (position >= value.size()) return value.size();
        ++position;
        while (position < value.size() &&
               (static_cast<unsigned char>(value[position]) & 0xC0) == 0x80) {
            ++position;
        }
        return position;
    }
}

bgui::inputbox::inputbox(const std::string& buffer, const std::string& placeholder, const float scale, std::function<void(const std::string)> action, bgui::input_mode mode) :
    linear(), m_placeholder(placeholder), m_input_buffer(buffer),
    m_cursor_position(buffer.size()), m_mode(mode), m_focused(false),
    m_backspace_down(false), m_backspace_next_time(0.f),
    m_cursor_blink_start(0.f),
    m_enter_func(action) {
    type = "inputarea";
    if (m_mode == input_mode::inputbox)
        style.layout.align = {alignment::start, alignment::center};
    recives_input(true);
    m_text = &add_persistent<text>(m_input_buffer.empty() ? m_placeholder : m_input_buffer, scale);
    m_text->add_class("inputarea-txt");
    m_text->recives_input(false);
}

void bgui::inputbox::set_min_float(float min) {
    m_min_float = min;
}

void bgui::inputbox::set_max_float(float max) {
    m_max_float = max;
}

void bgui::inputbox::set_float_callback(const std::function<void(float)>& c) {
    m_float_func = c;
}

bgui::inputbox::~inputbox() {
}

void bgui::inputbox::on_pressed() {
    set_focused(true);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::inputbox::on_clicked() {
    const float local_x = static_cast<float>(bgui::get_mouse_position().x - m_text->processed_x());
    size_t closest_position = 0;
    float closest_distance = std::abs(local_x);
    for (size_t position = 0;; position = next_utf8_character(m_input_buffer, position)) {
        const float text_width = m_text->get_text_width(m_input_buffer.substr(0, position));
        const float distance = std::abs(local_x - text_width);
        if (distance < closest_distance) {
            closest_position = position;
            closest_distance = distance;
        }
        if (position == m_input_buffer.size())
            break;
    }
    m_cursor_position = closest_position;
    set_focused(true);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::inputbox::on_released() {
    set_focused(true);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::inputbox::on_mouse_hover() {
    if (!m_focused) {
        set_style_state(state::hover);
    }
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

bgui::text& bgui::inputbox::get_label() {
    return *m_text;
}

void bgui::inputbox::on_update() {
    auto& ctx = bgui::get_context();

    if (m_focused) {
        if (!ctx.m_char_buffer.empty()) {
            m_input_buffer.insert(m_cursor_position, ctx.m_char_buffer);
            m_cursor_position += ctx.m_char_buffer.size();
            ctx.m_char_buffer.clear();
        }

        const auto backspace = ctx.m_input_map[bgui::input_key::backspace];
        const bool backspace_held =
            backspace == bgui::input_action::press ||
            backspace == bgui::input_action::repeat;
        const float now = bgui::get_time();
        if (!backspace_held) {
            m_backspace_down = false;
        } else if (!m_backspace_down) {
            erase_before_cursor();
            m_backspace_down = true;
            m_backspace_next_time = now + 0.35f;
        } else if (now >= m_backspace_next_time) {
            erase_before_cursor();
            m_backspace_next_time = now + 0.05f;
        }

        const auto left = ctx.m_input_map[bgui::input_key::left];
        if (left == bgui::input_action::press || left == bgui::input_action::repeat) {
            move_cursor_left();
        }

        const auto right = ctx.m_input_map[bgui::input_key::right];
        if (right == bgui::input_action::press || right == bgui::input_action::repeat) {
            move_cursor_right();
        }

        if (bgui::get_pressed(bgui::input_key::enter)) {
            switch (m_mode) {
            case input_mode::inputbox:
                if(m_enter_func)
                bgui::add_function([this]() {
                    m_enter_func(get_buffer());
                });
            break;
            case input_mode::number:
                auto v = get_buffer();
                if(v.empty()) break;
                float value = std::stof(v);
                float t = std::clamp(value, m_min_float, m_max_float);
                if(m_float_func)
                bgui::add_function([this, t]() {    
                    m_float_func(t);
                });
                auto str = std::to_string(t);
                str = remove_trailing_zeros(str);
                set_buffer(str);
            break;
            }
        }
    }

    linear::on_update();
    if (m_focused) {
        set_style_state(state::focused);
    }
    update_display();
}

void bgui::inputbox::update_display() {
    if (m_input_buffer.empty() && !m_focused) {
        m_text->set_buffer(m_placeholder);
        m_text->set_cursor(0, false);
        m_text->computed_style.visual.text.a = 0.4f;
        return;
    }

    const float blink_phase = std::fmod(
        bgui::get_time() - m_cursor_blink_start,
        1.0f
    );
    m_text->set_buffer(m_input_buffer);
    m_text->set_cursor(m_cursor_position, m_focused && blink_phase < 0.5f);
    m_text->computed_style.visual.text.a = 1.f;
}

void bgui::inputbox::set_focused(bool focused) {
    if (focused && !m_focused) {
        m_cursor_blink_start = bgui::get_time();
        m_backspace_down = false;
    }
    if (!focused) {
        m_backspace_down = false;
    }
    m_focused = focused;
    set_style_state(focused ? state::focused : state::normal);
}

void bgui::inputbox::move_cursor_left() {
    m_cursor_position = previous_utf8_character(m_input_buffer, m_cursor_position);
}

void bgui::inputbox::move_cursor_right() {
    m_cursor_position = next_utf8_character(m_input_buffer, m_cursor_position);
}

void bgui::inputbox::erase_before_cursor() {
    if (m_cursor_position == 0) return;
    const size_t previous = previous_utf8_character(m_input_buffer, m_cursor_position);
    m_input_buffer.erase(previous, m_cursor_position - previous);
    m_cursor_position = previous;
}

void bgui::inputbox::get_requires(bgui::draw_data* data) {
    linear::get_requires(data);
}
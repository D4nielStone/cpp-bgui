#include "elem/input_area.hpp"
#include "os/os.hpp"
#include "bgui.hpp"
#include <cmath>

namespace {
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

bgui::input_area::input_area(const std::string& buffer, const float scale, std::function<void(const std::string)> action, const std::string& placeholder, bgui::input_mode mode) :
    linear(), m_placeholder(placeholder), m_input_buffer(buffer),
    m_cursor_position(buffer.size()), m_mode(mode), m_focused(false),
    m_backspace_down(false), m_backspace_next_time(0.f),
    m_cursor_blink_start(0.f),
    m_enter_func(action) {
    type = "inputarea";
    recives_input(true);
    m_text = &add<text>(m_input_buffer.empty() ? m_placeholder : m_input_buffer, scale);
    m_text->add_class("inputarea-txt");
    m_text->recives_input(false);
}

bgui::input_area::~input_area() {
}

void bgui::input_area::on_pressed() {
    set_focused(true);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_clicked() {
    set_focused(true);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_released() {
    set_focused(true);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::input_area::on_mouse_hover() {
    if (!m_focused) {
        set_style_state(state::hover);
    }
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

bgui::text& bgui::input_area::get_label() {
    return *m_text;
}

void bgui::input_area::on_update() {
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

        if (m_mode == bgui::input_mode::inputbox && bgui::get_pressed(bgui::input_key::enter)) {
            bgui::add_function([this]() {
                m_enter_func(get_buffer());
                set_buffer("");
            });
        }
    }

    linear::on_update();
    if (m_focused) {
        set_style_state(state::focused);
    }
    update_display();
}

void bgui::input_area::update_display() {
    if (m_input_buffer.empty() && !m_focused) {
        m_text->set_buffer(m_placeholder);
        m_text->computed_style.visual.text.a = 0.4f;
        return;
    }

    std::string display = m_input_buffer;
    const float blink_phase = std::fmod(
        bgui::get_time() - m_cursor_blink_start,
        1.0f
    );
    if (m_focused && blink_phase < 0.5f) {
        display.insert(m_cursor_position, "|");
    }
    m_text->set_buffer(display);
    m_text->computed_style.visual.text.a = 1.f;
}

void bgui::input_area::set_focused(bool focused) {
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

void bgui::input_area::move_cursor_left() {
    m_cursor_position = previous_utf8_character(m_input_buffer, m_cursor_position);
}

void bgui::input_area::move_cursor_right() {
    m_cursor_position = next_utf8_character(m_input_buffer, m_cursor_position);
}

void bgui::input_area::erase_before_cursor() {
    if (m_cursor_position == 0) return;
    const size_t previous = previous_utf8_character(m_input_buffer, m_cursor_position);
    m_input_buffer.erase(previous, m_cursor_position - previous);
    m_cursor_position = previous;
}

void bgui::input_area::get_requires(bgui::draw_data* data) {
    linear::get_requires(data);
}
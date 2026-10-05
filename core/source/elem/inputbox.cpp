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

    size_t line_start(const std::string& value, size_t position) {
        const size_t newline = position == 0 ? std::string::npos : value.rfind('\n', position - 1);
        return newline == std::string::npos ? 0 : newline + 1;
    }

    size_t line_end(const std::string& value, size_t position) {
        const size_t newline = value.find('\n', position);
        return newline == std::string::npos ? value.size() : newline;
    }

    template<typename Move>
    void process_repeating_key(
        bgui::input_action action,
        float now,
        bool& was_down,
        float& next_repeat,
        Move move
    ) {
        const bool down = action == bgui::input_action::press ||
                          action == bgui::input_action::repeat;
        if (!down) {
            was_down = false;
            return;
        }
        if (!was_down) {
            move();
            was_down = true;
            next_repeat = now + 0.35f;
        } else if (now >= next_repeat) {
            move();
            next_repeat = now + 0.05f;
        }
    }
}

bgui::inputbox::inputbox(const std::string& buffer, const std::string& placeholder, const float scale, std::function<void(const std::string)> action, bgui::input_mode mode) :
    linear(), m_placeholder(placeholder), m_input_buffer(buffer),
    m_cursor_position(buffer.size()), m_mode(mode), m_focused(false),
    m_backspace_down(false), m_delete_down(false), m_enter_down(false),
    m_left_down(false), m_right_down(false), m_up_down(false), m_down_down(false),
    m_backspace_next_time(0.f), m_delete_next_time(0.f),
    m_left_next_time(0.f), m_right_next_time(0.f), m_up_next_time(0.f), m_down_next_time(0.f),
    m_cursor_blink_start(0.f),
    m_enter_func(action) {
    type = "inputarea";
    if (m_mode == input_mode::inputbox)
        style.layout.align = {alignment::start, alignment::center};
    recives_input(true);
    m_text = &add_persistent<text>(m_input_buffer.empty() ? m_placeholder : m_input_buffer, scale);
    m_text->add_class("inputarea-txt");
    m_text->recives_input(false);
    m_text->set_wrap(m_mode == input_mode::multiline);
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

void bgui::inputbox::set_highlight_rules(
    std::shared_ptr<const std::vector<syntax_highlight_rule>> rules
) {
    m_text->set_highlight_rules(std::move(rules));
}

bgui::inputbox::~inputbox() {
}

void bgui::inputbox::on_pressed() {
    set_focused(true);
    bgui::get_context().m_actual_cursor = bgui::cursor::ibeam;
}

void bgui::inputbox::on_clicked() {
    const auto mouse = bgui::get_mouse_position();
    if (m_mode == input_mode::multiline) {
        m_cursor_position = m_text->get_cursor_position_at(
            static_cast<float>(mouse.x), static_cast<float>(mouse.y));
    } else {
        const float local_x = static_cast<float>(mouse.x - m_text->processed_x());
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
    }
    m_cursor_blink_start = bgui::get_time();
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
            m_cursor_blink_start = bgui::get_time();
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

        const auto process_cursor_key = [this, now](
            input_action action,
            bool& was_down,
            float& next_repeat,
            auto move
        ) {
            const size_t previous_position = m_cursor_position;
            process_repeating_key(action, now, was_down, next_repeat, move);
            if (m_cursor_position != previous_position)
                m_cursor_blink_start = now;
        };
        process_cursor_key(
            ctx.m_input_map[bgui::input_key::left],
            m_left_down, m_left_next_time, [this]() { move_cursor_left(); });
        process_cursor_key(
            ctx.m_input_map[bgui::input_key::right],
            m_right_down, m_right_next_time, [this]() { move_cursor_right(); });
        if (m_mode == input_mode::multiline) {
            process_cursor_key(
                ctx.m_input_map[bgui::input_key::up],
                m_up_down, m_up_next_time, [this]() { move_cursor_up(); });
            process_cursor_key(
                ctx.m_input_map[bgui::input_key::down],
                m_down_down, m_down_next_time, [this]() { move_cursor_down(); });
        } else {
            m_up_down = false;
            m_down_down = false;
        }

        const auto delete_action = ctx.m_input_map[bgui::input_key::delete_key];
        const bool delete_held =
            delete_action == bgui::input_action::press ||
            delete_action == bgui::input_action::repeat;
        if (!delete_held) {
            m_delete_down = false;
        } else if (!m_delete_down) {
            erase_at_cursor();
            m_delete_down = true;
            m_delete_next_time = now + 0.35f;
        } else if (now >= m_delete_next_time) {
            erase_at_cursor();
            m_delete_next_time = now + 0.05f;
        }

        const auto enter = ctx.m_input_map[bgui::input_key::enter];
        const auto keypad_enter = ctx.m_input_map[bgui::input_key::keypad_enter];
        const bool enter_down =
            enter == bgui::input_action::press || enter == bgui::input_action::repeat ||
            keypad_enter == bgui::input_action::press || keypad_enter == bgui::input_action::repeat;
        if (enter_down && !m_enter_down) {
            if (m_mode == input_mode::multiline) {
                m_input_buffer.insert(m_cursor_position, "\n");
                ++m_cursor_position;
                m_cursor_blink_start = now;
            } else {
                switch (m_mode) {
                case input_mode::inputbox:
                    if (m_enter_func)
                        bgui::add_function([this]() {
                            m_enter_func(get_buffer());
                        });
                    break;
                case input_mode::number: {
                    auto value_buffer = get_buffer();
                    if (value_buffer.empty()) break;
                    const float value = std::stof(value_buffer);
                    const float clamped = std::clamp(value, m_min_float, m_max_float);
                    if (m_float_func)
                        bgui::add_function([this, clamped]() {
                            m_float_func(clamped);
                        });
                    auto formatted = std::to_string(clamped);
                    formatted = remove_trailing_zeros(formatted);
                    set_buffer(formatted);
                    break;
                }
                default:
                    break;
                }
            }
        }
        m_enter_down = enter_down;
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
        m_delete_down = false;
    }
    if (!focused) {
        m_backspace_down = false;
        m_delete_down = false;
        m_enter_down = false;
        m_left_down = false;
        m_right_down = false;
        m_up_down = false;
        m_down_down = false;
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

void bgui::inputbox::move_cursor_up() {
    const size_t visual_position = m_text->get_vertical_cursor_position(m_cursor_position, -1);
    if (visual_position != m_cursor_position) {
        m_cursor_position = visual_position;
        return;
    }
    const size_t current_start = line_start(m_input_buffer, m_cursor_position);
    if (current_start == 0) return;

    const size_t previous_end = current_start - 1;
    const size_t previous_start = line_start(m_input_buffer, previous_end);
    size_t column = 0;
    for (size_t position = current_start; position < m_cursor_position;
         position = next_utf8_character(m_input_buffer, position)) {
        ++column;
    }

    m_cursor_position = previous_start;
    while (column > 0 && m_cursor_position < previous_end) {
        m_cursor_position = next_utf8_character(m_input_buffer, m_cursor_position);
        --column;
    }
}

void bgui::inputbox::move_cursor_down() {
    const size_t visual_position = m_text->get_vertical_cursor_position(m_cursor_position, 1);
    if (visual_position != m_cursor_position) {
        m_cursor_position = visual_position;
        return;
    }
    const size_t current_start = line_start(m_input_buffer, m_cursor_position);
    const size_t current_end = line_end(m_input_buffer, m_cursor_position);
    if (current_end == m_input_buffer.size()) return;

    size_t column = 0;
    for (size_t position = current_start; position < m_cursor_position;
         position = next_utf8_character(m_input_buffer, position)) {
        ++column;
    }

    const size_t next_start = current_end + 1;
    const size_t next_end = line_end(m_input_buffer, next_start);
    m_cursor_position = next_start;
    while (column > 0 && m_cursor_position < next_end) {
        m_cursor_position = next_utf8_character(m_input_buffer, m_cursor_position);
        --column;
    }
}

void bgui::inputbox::erase_before_cursor() {
    if (m_cursor_position == 0) return;
    const size_t previous = previous_utf8_character(m_input_buffer, m_cursor_position);
    m_input_buffer.erase(previous, m_cursor_position - previous);
    m_cursor_position = previous;
    m_cursor_blink_start = bgui::get_time();
}

void bgui::inputbox::erase_at_cursor() {
    if (m_cursor_position >= m_input_buffer.size()) return;
    const size_t next = next_utf8_character(m_input_buffer, m_cursor_position);
    m_input_buffer.erase(m_cursor_position, next - m_cursor_position);
    m_cursor_blink_start = bgui::get_time();
}

void bgui::inputbox::get_requires(bgui::draw_data* data) {
    linear::get_requires(data);
}
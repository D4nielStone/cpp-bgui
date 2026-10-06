#include "elem/inputbox.hpp"
#include "os/os.hpp"
#include "bgui.hpp"
#include <cctype>
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

    bool is_word_character(const std::string& value, size_t position) {
        const auto character = static_cast<unsigned char>(value[position]);
        return character >= 0x80 || std::isalnum(character) || character == '_';
    }

    bool is_valid_number_edit(const std::string& value) {
        bool has_decimal_point = false;
        for (size_t i = 0; i < value.size(); ++i) {
            const char character = value[i];
            if (character >= '0' && character <= '9')
                continue;
            if ((character == '-' || character == '+') && i == 0)
                continue;
            if (character == '.' && !has_decimal_point) {
                has_decimal_point = true;
                continue;
            }
            return false;
        }
        return true;
    }

    size_t previous_word_boundary(const std::string& value, size_t position) {
        while (position > 0) {
            const size_t previous = previous_utf8_character(value, position);
            if (is_word_character(value, previous))
                break;
            position = previous;
        }
        while (position > 0) {
            const size_t previous = previous_utf8_character(value, position);
            if (!is_word_character(value, previous))
                break;
            position = previous;
        }
        return position;
    }

    size_t next_word_boundary(const std::string& value, size_t position) {
        while (position < value.size() && !is_word_character(value, position))
            position = next_utf8_character(value, position);
        while (position < value.size() && is_word_character(value, position))
            position = next_utf8_character(value, position);
        while (position < value.size() && !is_word_character(value, position))
            position = next_utf8_character(value, position);
        return position;
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
    m_cursor_position(buffer.size()), m_selection_anchor(std::numeric_limits<size_t>::max()),
    m_mode(mode), m_focused(false),
    m_backspace_down(false), m_delete_down(false), m_enter_down(false),
    m_left_down(false), m_right_down(false), m_up_down(false), m_down_down(false),
    m_home_down(false), m_end_down(false), m_select_all_down(false),
    m_copy_down(false), m_cut_down(false), m_paste_down(false), m_tab_down(false),
    m_backspace_next_time(0.f), m_delete_next_time(0.f),
    m_left_next_time(0.f), m_right_next_time(0.f), m_up_next_time(0.f), m_down_next_time(0.f),
    m_home_next_time(0.f), m_end_next_time(0.f),
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
    clear_selection();
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
        const auto key_action = [&ctx](input_key key) {
            return ctx.m_input_map[key];
        };
        const bool control_down =
            key_action(input_key::left_control) == input_action::press ||
            key_action(input_key::left_control) == input_action::repeat ||
            key_action(input_key::right_control) == input_action::press ||
            key_action(input_key::right_control) == input_action::repeat;
        const bool shift_down =
            key_action(input_key::left_shift) == input_action::press ||
            key_action(input_key::left_shift) == input_action::repeat ||
            key_action(input_key::right_shift) == input_action::press ||
            key_action(input_key::right_shift) == input_action::repeat;

        if (m_mode == input_mode::multiline) {
            const bool select_all_down = control_down &&
                key_action(input_key::a) == input_action::press;
            if (select_all_down && !m_select_all_down) {
                m_selection_anchor = 0;
                m_cursor_position = m_input_buffer.size();
                m_cursor_blink_start = bgui::get_time();
            }
            m_select_all_down = select_all_down;

            const bool copy_down = control_down &&
                key_action(input_key::c) == input_action::press;
            if (copy_down && !m_copy_down && has_selection() && ctx.m_set_clipboard) {
                ctx.m_set_clipboard(m_input_buffer.substr(
                    selection_start(), selection_end() - selection_start()));
            }
            m_copy_down = copy_down;

            const bool cut_down = control_down &&
                key_action(input_key::x) == input_action::press;
            if (cut_down && !m_cut_down && has_selection() && ctx.m_set_clipboard) {
                ctx.m_set_clipboard(m_input_buffer.substr(
                    selection_start(), selection_end() - selection_start()));
                erase_selection();
            }
            m_cut_down = cut_down;

            const bool paste_down = control_down &&
                (key_action(input_key::v) == input_action::press ||
                 key_action(input_key::v) == input_action::repeat);
            if (paste_down && !m_paste_down && ctx.m_get_clipboard) {
                const std::string clipboard = ctx.m_get_clipboard();
                erase_selection();
                m_input_buffer.insert(m_cursor_position, clipboard);
                m_cursor_position += clipboard.size();
                m_cursor_blink_start = bgui::get_time();
            }
            m_paste_down = paste_down;
        } else {
            m_select_all_down = false;
            m_copy_down = false;
            m_cut_down = false;
            m_paste_down = false;
        }

        if (!ctx.m_char_buffer.empty()) {
            if (m_mode == input_mode::number) {
                const size_t start = has_selection() ? selection_start() : m_cursor_position;
                const size_t end = has_selection() ? selection_end() : m_cursor_position;
                std::string candidate = m_input_buffer;
                candidate.erase(start, end - start);
                size_t insertion_position = start;
                for (char character : ctx.m_char_buffer) {
                    candidate.insert(insertion_position, 1, character);
                    if (is_valid_number_edit(candidate)) {
                        ++insertion_position;
                    } else {
                        candidate.erase(insertion_position, 1);
                    }
                }
                if (insertion_position != start || !has_selection()) {
                    m_input_buffer = std::move(candidate);
                    m_cursor_position = insertion_position;
                    clear_selection();
                }
            } else {
                erase_selection();
                m_input_buffer.insert(m_cursor_position, ctx.m_char_buffer);
                m_cursor_position += ctx.m_char_buffer.size();
            }
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
            if (!erase_selection()) {
                if (control_down)
                    erase_word_before_cursor();
                else
                    erase_before_cursor();
            }
            m_backspace_down = true;
            m_backspace_next_time = now + 0.35f;
        } else if (now >= m_backspace_next_time) {
            if (!erase_selection()) {
                if (control_down)
                    erase_word_before_cursor();
                else
                    erase_before_cursor();
            }
            m_backspace_next_time = now + 0.05f;
        }

        const auto process_cursor_key = [this, now, shift_down](
            input_action action,
            bool& was_down,
            float& next_repeat,
            input_key key,
            auto move
        ) {
            const size_t previous_position = m_cursor_position;
            const bool extend = m_mode == input_mode::multiline && shift_down;
            process_repeating_key(action, now, was_down, next_repeat, [this, extend, key, move]() {
                if (extend) {
                    if (m_selection_anchor == std::numeric_limits<size_t>::max())
                        m_selection_anchor = m_cursor_position;
                    move();
                } else if (has_selection()) {
                    m_cursor_position =
                        (key == input_key::left || key == input_key::up)
                        ? selection_start() : selection_end();
                    clear_selection();
                } else {
                    move();
                    clear_selection();
                }
            });
            if (m_cursor_position != previous_position)
                m_cursor_blink_start = now;
        };
        process_cursor_key(
            ctx.m_input_map[bgui::input_key::left],
            m_left_down, m_left_next_time, input_key::left,
            [this, control_down]() {
                if (control_down)
                    move_cursor_word_left();
                else
                    move_cursor_left();
            });
        process_cursor_key(
            ctx.m_input_map[bgui::input_key::right],
            m_right_down, m_right_next_time, input_key::right,
            [this, control_down]() {
                if (control_down)
                    move_cursor_word_right();
                else
                    move_cursor_right();
            });
        if (m_mode == input_mode::multiline) {
            process_cursor_key(
                ctx.m_input_map[bgui::input_key::up],
                m_up_down, m_up_next_time, input_key::up,
                [this]() { move_cursor_up(); });
            process_cursor_key(
                ctx.m_input_map[bgui::input_key::down],
                m_down_down, m_down_next_time, input_key::down,
                [this]() { move_cursor_down(); });
        } else {
            m_up_down = false;
            m_down_down = false;
        }
        const auto process_line_key = [this, &process_cursor_key](
            input_key key, bool& was_down, float& next_repeat, bool to_start
        ) {
            process_cursor_key(
                bgui::get_context().m_input_map[key], was_down, next_repeat, key,
                [this, to_start]() {
                    auto& input = bgui::get_context().m_input_map;
                    const bool document = input[input_key::left_control] == input_action::press ||
                        input[input_key::left_control] == input_action::repeat ||
                        input[input_key::right_control] == input_action::press ||
                        input[input_key::right_control] == input_action::repeat;
                    if (to_start)
                        move_cursor_to_line_start(document);
                    else
                        move_cursor_to_line_end(document);
                });
        };
        process_line_key(input_key::home, m_home_down, m_home_next_time, true);
        process_line_key(input_key::end, m_end_down, m_end_next_time, false);

        const auto tab_action = ctx.m_input_map[input_key::tab];
        const bool tab_held =
            tab_action == input_action::press || tab_action == input_action::repeat;
        if (m_mode == input_mode::multiline && tab_held && !m_tab_down) {
            erase_selection();
            m_input_buffer.insert(m_cursor_position, "\t");
            ++m_cursor_position;
            m_cursor_blink_start = now;
        }
        m_tab_down = m_mode == input_mode::multiline && tab_held;

        const auto delete_action = ctx.m_input_map[bgui::input_key::delete_key];
        const bool delete_held =
            delete_action == bgui::input_action::press ||
            delete_action == bgui::input_action::repeat;
        if (!delete_held) {
            m_delete_down = false;
        } else if (!m_delete_down) {
            if (!erase_selection()) {
                if (control_down)
                    erase_word_at_cursor();
                else
                    erase_at_cursor();
            }
            m_delete_down = true;
            m_delete_next_time = now + 0.35f;
        } else if (now >= m_delete_next_time) {
            if (!erase_selection()) {
                if (control_down)
                    erase_word_at_cursor();
                else
                    erase_at_cursor();
            }
            m_delete_next_time = now + 0.05f;
        }

        const auto enter = ctx.m_input_map[bgui::input_key::enter];
        const auto keypad_enter = ctx.m_input_map[bgui::input_key::keypad_enter];
        const bool enter_down =
            enter == bgui::input_action::press || enter == bgui::input_action::repeat ||
            keypad_enter == bgui::input_action::press || keypad_enter == bgui::input_action::repeat;
        if (enter_down && !m_enter_down) {
            if (m_mode == input_mode::multiline) {
                erase_selection();
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
        m_text->set_selection(0, 0);
        m_text->set_cursor(0, false);
        m_text->computed_style.visual.text.a = 0.4f;
        return;
    }

    const float blink_phase = std::fmod(
        bgui::get_time() - m_cursor_blink_start,
        1.0f
    );
    m_text->set_buffer(m_input_buffer);
    m_text->set_selection(
        has_selection() ? selection_start() : m_cursor_position,
        has_selection() ? selection_end() : m_cursor_position);
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
        m_home_down = false;
        m_end_down = false;
        m_select_all_down = false;
        m_copy_down = false;
        m_cut_down = false;
        m_paste_down = false;
        m_tab_down = false;
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

void bgui::inputbox::move_cursor_word_left() {
    m_cursor_position = previous_word_boundary(m_input_buffer, m_cursor_position);
}

void bgui::inputbox::move_cursor_word_right() {
    m_cursor_position = next_word_boundary(m_input_buffer, m_cursor_position);
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

void bgui::inputbox::erase_word_before_cursor() {
    if (m_cursor_position == 0)
        return;
    const size_t start = previous_word_boundary(m_input_buffer, m_cursor_position);
    m_input_buffer.erase(start, m_cursor_position - start);
    m_cursor_position = start;
    m_cursor_blink_start = bgui::get_time();
}

void bgui::inputbox::erase_word_at_cursor() {
    if (m_cursor_position >= m_input_buffer.size())
        return;
    const size_t end = next_word_boundary(m_input_buffer, m_cursor_position);
    m_input_buffer.erase(m_cursor_position, end - m_cursor_position);
    m_cursor_blink_start = bgui::get_time();
}

bool bgui::inputbox::erase_selection() {
    if (!has_selection()) {
        clear_selection();
        return false;
    }
    const size_t start = selection_start();
    m_input_buffer.erase(start, selection_end() - start);
    m_cursor_position = start;
    clear_selection();
    m_cursor_blink_start = bgui::get_time();
    return true;
}

void bgui::inputbox::move_cursor_to_line_start(bool document_start) {
    m_cursor_position = document_start ? 0 : line_start(m_input_buffer, m_cursor_position);
}

void bgui::inputbox::move_cursor_to_line_end(bool document_end) {
    m_cursor_position = document_end
        ? m_input_buffer.size() : line_end(m_input_buffer, m_cursor_position);
}

bool bgui::inputbox::has_selection() const {
    return m_selection_anchor != std::numeric_limits<size_t>::max() &&
           m_selection_anchor != m_cursor_position;
}

size_t bgui::inputbox::selection_start() const {
    return std::min(m_selection_anchor, m_cursor_position);
}

size_t bgui::inputbox::selection_end() const {
    return std::max(m_selection_anchor, m_cursor_position);
}

void bgui::inputbox::clear_selection() {
    m_selection_anchor = std::numeric_limits<size_t>::max();
}

void bgui::inputbox::get_requires(bgui::draw_data* data) {
    linear::get_requires(data);
}
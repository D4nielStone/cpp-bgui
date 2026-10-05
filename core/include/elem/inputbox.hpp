#pragma once
#include "lay/linear.hpp"
#include "text.hpp"
#include "utils/syntax_highlight.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace bgui {
    enum class input_mode {
        inputbox,
        number,
        multiline
    };

    /// \brief A text input element.
    /// It allows the user to input text, number ...
    class inputbox : public linear {
    protected:
        text* m_text;
        std::string m_placeholder, m_input_buffer;
        size_t m_cursor_position;
        input_mode m_mode;
        bool m_focused;
        bool m_backspace_down;
        bool m_delete_down;
        bool m_enter_down;
        bool m_left_down;
        bool m_right_down;
        bool m_up_down;
        bool m_down_down;
        float m_backspace_next_time;
        float m_delete_next_time;
        float m_left_next_time;
        float m_right_next_time;
        float m_up_next_time;
        float m_down_next_time;
        float m_cursor_blink_start;
        float m_min_float;
        float m_max_float;

        void update_display();
        void move_cursor_left();
        void move_cursor_right();
        void move_cursor_up();
        void move_cursor_down();
        void erase_before_cursor();
        void erase_at_cursor();
    public:
        std::function<void(const std::string)> m_enter_func = nullptr;
        std::function<void(const float)> m_float_func = nullptr;
        /// \brief Contructor.
        /// \param buffer The initial text to inject on the buffer.
        /// \param scale The scale of the text.
        /// \param placeholder The message to display when the buffer is empty.
        explicit inputbox(const std::string& buffer, const std::string& placeholder = "", const float scale = 1.f, std::function<void(const std::string)> action = nullptr, input_mode mode = input_mode::inputbox);
        ~inputbox();
        
        void set_min_float(float f);
        void set_max_float(float f);
        void set_float_callback(const std::function<void(const float)>& f);
        void set_highlight_rules(
            std::shared_ptr<const std::vector<syntax_highlight_rule>> rules
        );
        void set_wrap(bool enabled) { m_text->set_wrap(enabled); }
        bool is_wrap_enabled() const { return m_text->is_wrap_enabled(); }
        void on_clicked() override;
        void on_pressed() override;
        void on_released() override;
        void on_mouse_hover() override;
        void on_update() override;
        text& get_label();
        void get_requires(bgui::draw_data* calls) override;
        bool clips_children() const override { return true; }
        void set_buffer(const std::string& buffer) {
            m_input_buffer = buffer;
            m_cursor_position = m_input_buffer.size();
            update_display();
        }
        std::string& get_buffer() { return m_input_buffer; }
        const std::string& get_buffer() const { return m_input_buffer; }
        input_mode get_input_mode() const { return m_mode; }
        void set_input_mode(input_mode mode) { m_mode = mode; }
        size_t get_cursor_position() const { return m_cursor_position; }
        void set_focused(bool focused);
        bool is_focused() const { return m_focused; }
    };
} // namespace bgui
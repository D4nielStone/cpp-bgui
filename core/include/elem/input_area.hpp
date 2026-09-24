#pragma once
#include "lay/linear.hpp"
#include "text.hpp"
#include <functional>

namespace bgui {
    enum class input_mode {
        inputbox,
        multiline
    };

    /// \brief A text input element.
    /// It allows the user to input text, number ...
    class input_area : public linear {
    private:
        text* m_text;
        std::string m_placeholder, m_input_buffer;
        size_t m_cursor_position;
        input_mode m_mode;
        bool m_focused;
        bool m_backspace_down;
        float m_backspace_next_time;
        float m_cursor_blink_start;

        void update_display();
        void move_cursor_left();
        void move_cursor_right();
        void erase_before_cursor();
    public:
        std::function<void(const std::string)> m_enter_func;
        /// \brief Contructor.
        /// \param buffer The initial text to inject on the buffer.
        /// \param scale The scale of the text.
        /// \param placeholder The message to display when the buffer is empty.
        explicit input_area(const std::string& buffer, const float scale, std::function<void(const std::string)> action, const std::string& placeholder = "", input_mode mode = input_mode::inputbox);
        ~input_area();
        
        void on_clicked() override;
        void on_pressed() override;
        void on_released() override;
        void on_mouse_hover() override;
        void on_update() override;
        text& get_label();
        void get_requires(bgui::draw_data* calls) override;
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
#pragma once
#include "lay/linear.hpp"
#include "text.hpp"
#include <functional>

namespace bgui {
    /// \brief A text input element.
    /// It allows the user to input text, number ...
    class input_area : public linear {
    private:
        text* m_text;
        std::string m_placeholder, m_input_buffer;
    public:
        /// \brief Contructor.
        /// \param buffer The initial text to inject on the buffer.
        /// \param scale The scale of the text.
        /// \param placeholder The message to display when the buffer is empty.
        explicit input_area(const std::string& buffer, const float scale, const std::string& placeholder = "");
        ~input_area();
        
        void on_clicked() override;
        void on_pressed() override;
        void on_released() override;
        void on_mouse_hover() override;
        void on_update() override;
        text& get_label();
        void get_requests(bgui::draw_data* calls) override;
        void apply_style(const bgui::style& t) override;
    };
} // namespace bgui
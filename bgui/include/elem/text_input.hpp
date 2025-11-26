#pragma once
#include "buttom.hpp"
#include <functional>

namespace elements {
    /// \brief A text input element.
    /// It allows the user to input text.
    class text_input : public element {
    private:
        text m_buffer;
        buttom m_buttom;
        std::string m_placeholder;
    public:
        /// \brief Contructor.
        /// \param buffer The initial text to inject on the buffer.
        /// \param scale The scale of the text.
        /// \param buttom_label The label of the button.
        /// \param f The function callback. Is called when the button is clicked.
        /// \param placeholder The message to display when the buffer is empty.
        explicit text_input(const std::string& buffer, const float scale, const std::string& buttom_label = "done",
            const std::function<void()>& f = nullptr,
            const std::string& placeholder = "");
        ~text_input();
        
        void on_clicked() override;
        void on_released() override;
        void on_mouse_hover() override;
        void update() override;
        void get_draw_requests(std::vector<butil::draw_request>& calls) override;
        void apply_theme(const butil::theme& t) override;
    };
} // namespace elements
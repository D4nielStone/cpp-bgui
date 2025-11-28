#pragma once
#include "text.hpp"
#include <functional>

namespace belem {
    /// \brief A text input element.
    /// It allows the user to input text.
    class text_input : public element {
    private:
        text m_buffer;
        std::string m_placeholder;
    public:
        /// \brief Contructor.
        /// \param buffer The initial text to inject on the buffer.
        /// \param scale The scale of the text.
        /// \param placeholder The message to display when the buffer is empty.
        explicit text_input(const std::string& buffer, const float scale, const std::string& placeholder = "");
        ~text_input();
        
        void on_clicked() override;
        void on_released() override;
        void on_mouse_hover() override;
        void update() override;
        void get_requests(butil::draw_data& calls) override;
        void apply_theme(const butil::theme& t) override;
    };
} // namespace belem
#include "elem/details.hpp"

namespace bgui {
    details::details(const std::string& summary, const bool open)
        : linear(orientation::vertical), m_summary(summary) {
        type = "details";
        style.layout.require_mode(mode::match_parent, mode::wrap_content);
        m_summary_button = &add_persistent<button>("", 0.3f, [this]() {
            set_open(!m_open);
        });
        m_summary_button->style.layout.require_mode(mode::match_parent, mode::wrap_content);
        m_summary_button->add_class("details-summary");
        m_content = &add_persistent<linear>(orientation::vertical);
        m_content->style.layout.require_mode(mode::match_parent, mode::wrap_content);
        m_content->add_class("details-content");
        set_open(open);
    }

    void details::set_open(const bool open) {
        m_open = open;
        m_content->set_enable(open);
        m_summary_button->get_label().set_buffer(
            std::string(open ? "- " : "+ ") + m_summary
        );
    }
}
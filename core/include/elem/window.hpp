#pragma once
#include "lay/linear.hpp"
#include "elem/text.hpp"
#include <string>

namespace bgui {
    class window : public linear {
    private:
        text* m_title;
        linear* m_header;
        bool m_floating = true;
    public:
        window() = default;
        window(const char* title, bool floating = true);
        ~window()=default;
        void on_update() override;
        void set_title(const std::string& title) { m_title->set_buffer(title); }
        text& get_title() { return *m_title; }
        bool is_floating() const { return m_floating; }
        void set_floating(bool floating);
    };
}// namespace bgui
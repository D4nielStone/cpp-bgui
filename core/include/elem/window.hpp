#pragma once
#include "lay/linear.hpp"
#include "elem/text.hpp"
#include <string>

namespace bgui {
    class window : public linear {
    private:
        text* m_title;
        linear* m_header;
    public:
        window() = default;
        window(const char* title);
        ~window()=default;
        void on_update() override;
        void set_title(const std::string& title) { m_title->set_buffer(title); }
        text& get_title() { return *m_title; }
    };
}// namespace bgui
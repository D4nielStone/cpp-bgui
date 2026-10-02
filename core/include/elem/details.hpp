#pragma once

#include "elem/button.hpp"

namespace bgui {
    class details : public linear {
    public:
        explicit details(const std::string& summary, bool open = false);

        void set_open(bool open);
        bool is_open() const { return m_open; }
        linear& content() { return *m_content; }

    private:
        std::string m_summary;
        button* m_summary_button{nullptr};
        linear* m_content{nullptr};
        bool m_open{false};
    };
}
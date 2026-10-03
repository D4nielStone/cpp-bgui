#pragma once
#include "lay/linear.hpp"
#include "elem/image.hpp"
#include "elem/text.hpp"
#include <string>

namespace bgui {
    class button;

    class window : public linear {
    private:
        text* m_title{nullptr};
        linear* m_header{nullptr};
        image* m_icon{nullptr};
        button* m_close_button{nullptr};
        float m_icon_aspect_ratio = 1.f;
        int m_icon_height = 0;
        vec2i m_pinned_drag_distance{0, 0};
        bool m_floating = true;
        bool m_dragging = false;
    public:
        window() = default;
        window(const char* title, bool floating = true);
        ~window()=default;
        void on_update() override;
        void set_icon(const std::string& path);
        void set_title(const std::string& title) { m_title->set_buffer(title); }
        text& get_title() { return *m_title; }
        bool clips_children() const override { return true; }
        bool is_floating() const { return m_floating; }
        bool is_dragging() const { return m_dragging; }
        void set_floating(bool floating);
    };
}// namespace bgui
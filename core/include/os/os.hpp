#pragma once
#include <unordered_map>
#include <string>
#include "utils/vec.hpp"
#include "utils/mat.hpp"
#include "utils/enums.hpp"
#include <functional>

namespace bgui {
    struct drag_info {
        bool dragging;
        vec2 mouse_screen;
        vec2 start_mouse_screen;
        vec2 start_window_pos;
        vec2 offset;
    };
    struct context {
        drag_info m_draginfo;
        bgui::vec2i m_size{800, 600};
        bgui::vec2i m_mouse_position{0, 0};
        bgui::vec2i m_last_mouse_pos{0, 0};
        std::unordered_map<input_key, input_action> m_input_map;
        std::string m_char_buffer;
        std::string m_title{""};
        bool m_last_mouse_left = false;
        cursor m_actual_cursor = cursor::arrow;
        std::function<void()> m_refresh_func;
    };
    context& get_context();
    bgui::vec2i get_context_size();
    bgui::vec2i get_mouse_position();
    std::string read_file(const std::string& path);
    bgui::mat4 get_projection();
    bool get_pressed(const input_key&);
    float get_time();
    int get_fps();
} // namespace bgui
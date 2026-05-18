#include <fstream>
#include <sstream>
#include <iostream>
#include "os/os.hpp"
#include <chrono>

static bgui::context s_window_io;

bgui::context &bgui::get_context() {
    return s_window_io;
}
bgui::vec2i bgui::get_context_size() {
    return get_context().m_size;
}
bgui::vec2i bgui::get_mouse_position() {
    return bgui::get_context().m_mouse_position;
}
std::string bgui::read_file(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path + "\n");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bgui::mat4 bgui::get_projection() {
    auto size = bgui::get_context_size();
    auto width = size[0];
    auto height = size[1];
    return bgui::orthographic(0.0f, static_cast<float>(width), static_cast<float>(height), 0.f);
}

bool bgui::get_pressed(const bgui::input_key& k) {
    if(bgui::get_context().m_input_map[k] == bgui::input_action::press) return true;
    return false;
}

float bgui::get_time() {
    using namespace std::chrono;
    static auto start_time = high_resolution_clock::now();
    auto now = high_resolution_clock::now();
    duration<float> elapsed = now - start_time;
    return elapsed.count();
}

int bgui::get_fps() {
    static int frame_count = 0;
    static auto last_time = std::chrono::high_resolution_clock::now();
    frame_count++;
    static int fps = 0;
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = current_time - last_time;

    if (elapsed.count() >= 1.0f) {
        fps = frame_count / elapsed.count();
        frame_count = 0;
        last_time = current_time;
    }
    return fps;
}
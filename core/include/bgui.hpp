#pragma once
#include "elem/element.hpp"
#include "elem/button.hpp"
#include "elem/checkbox.hpp"
#include "elem/text.hpp"
#include "elem/window.hpp"
#include "elem/input_area.hpp"
#include "lay/layout.hpp"
#include "lay/modular.hpp"
#include "lay/linear.hpp"
#include "utils/mat.hpp"
#include "utils/vec.hpp"
#include "utils/style.hpp"
#include <queue>
#include <functional>

#ifdef BGUI_USE_GLFW
    #include "backend/bgui_backend_glfw.hpp"
#endif
#ifdef BGUI_USE_OPENGL
    #include "backend/bgui_backend_gl3.hpp"
#elif defined(BGUI_USE_VULKAN)
    #include "backend/bgui_backend_vulkan.hpp"
#endif
#ifdef BGUI_USE_FREETYPE
    #include "backend/bgui_backend_freetype.hpp"
#endif

namespace bgui {
    extern std::unique_ptr<layout> m_main_layout;

    template<typename T, typename... Args>
    T& set_layout(Args&&... args) {
        m_main_layout = std::make_unique<T>(std::forward<Args>(args)...);
        static_assert(std::is_base_of<layout, T>::value, "BGUI::the class T must be a layout type.");
        return static_cast<T&>(*m_main_layout);
    }

    void add_function(const std::function<void()>& f);
    layout& get_layout();
    void apply_style(const bgui::style& gui_style);
    bgui::style& get_style();
    bgui::draw_data* get_draw_data();
    void set_up();
    bool shutdown_lib();
    void update();
};
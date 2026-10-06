#pragma once
#include "os/os.hpp"
#include "os/font.hpp"
#include "os/asset_manager.hpp"
#include "elem/element.hpp"
#include "elem/field.hpp"
#include "elem/button.hpp"
#include "elem/checkbox.hpp"
#include "elem/menu_bar.hpp"
#include "elem/slider.hpp"
#include "elem/color_picker.hpp"
#include "elem/progress_bar.hpp"
#include "elem/details.hpp"
#include "elem/text.hpp"
#include "elem/image.hpp"
#include "elem/window.hpp"
#include "elem/inputbox.hpp"
#include "elem/modal.hpp"
#include "lay/layout.hpp"
#include "lay/dock.hpp"
#include "lay/modular.hpp"
#include "lay/linear.hpp"
#include "utils/mat.hpp"
#include "utils/vec.hpp"
#include "utils/draw.hpp"
#include "utils/logging.hpp"
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
    extern std::unique_ptr<layout> s_main_layout;

    template<typename T, typename... Args>
    T& set_layout(Args&&... args) {
        if (s_main_layout)
            cancel_interactions(s_main_layout.get());
        s_main_layout = std::make_unique<T>(std::forward<Args>(args)...);
        static_assert(std::is_base_of<layout, T>::value, "[BGUI] the class T must be a layout type.");
        return static_cast<T&>(*s_main_layout);
    }

    void add_function(const std::function<void()>& f);
    layout& get_layout();
    void cascade_style();
    bgui::draw_data* get_draw_data();
    bgui::draw_list& get_draw_list();
    void set_up();
    bool shutdown_lib();
    bool load_configuration(const std::string& path);
    bool save_configuration(const std::string& path);

    // RAII interface for BGUI context. This ensures that the BGUI context is properly initialized and cleaned up within a scope.
    class scoped_interface {
    public:
        scoped_interface();
        ~scoped_interface() noexcept;

        scoped_interface(const scoped_interface&) = delete;
        scoped_interface& operator=(const scoped_interface&) = delete;
        scoped_interface(scoped_interface&&) = delete;
        scoped_interface& operator=(scoped_interface&&) = delete;
    };

    void on_update();
    element* get_mouse_target();
    void clear_keyboard_focus();
    void set_global_scale(float scale);
    float get_global_scale();
};
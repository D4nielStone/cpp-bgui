/* \brief The bgui manages the graphic user interface.
    Here, the gui will be responsible for rendering and updating the elements.
    It includes:
    - Theme management (light/dark and maybe theme customization in the future)
    - Update:
        - [] Propagate the projection matrix to all elements
        - [] Update the logic of all elements base in a layout.
        - [] Decide the z_index of each element for rendering order. It's based on
         the order of addition to the parent (last added is on top). and the priority level: 
         normal elements < modal.
        - [] Types of layouts to be supported:
            - [] Absolute layout (uses the x, y, width, height directly)
            - [] Relative layout (percentage based positioning and sizing modular to parent)
            - [] Linear layout (horizontal/vertical)
            - [] Grid layout (organize elements in a grid with a `grid_size_rule` to define how the grid cells are sized)
        - [] Update the style of the elements base on the current theme
        - [] Inputs:
            - [] Elements who are "touchable" will handle mouse inputs based in the priority order.
            - [] Elements with higher z-index will have the priority to handle the inputs first.
            - [] Keyboard inputs will be handled by the focused element.
    -  Render:
        - [] Build the buffer of quad based on the main opengl context. 
        - [] Clear the screen with the bgui clear color
        - [] Propagate the projection matrix to all elements
        - [] Render all elements base in a z-index order.
        - [] Optimize rendering with techniques like:
            - [] Culling: Skip rendering elements that are outside the viewport.
            - [] Batching: Group similar draw calls to reduce state changes.
*/

#pragma once
#include "elem/element.hpp"
#include "elem/button.hpp"
#include "elem/text.hpp"
#include "lay/layout.hpp"
#include "lay/modular.hpp"
#include "lay/linear.hpp"
#include "utils/mat.hpp"
#include "utils/vec.hpp"
#include "utils/theme.hpp"
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
    void apply_theme(const bgui::theme& gui_theme);
    bgui::theme& get_theme();
    bgui::draw_data* get_draw_data();
    void set_up();
    bool shutdown_lib();
    void update();
};
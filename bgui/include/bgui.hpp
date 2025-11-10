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
            - [] Relative layout (percentage based positioning and sizing relative to parent)
            - [] Linear layout (horizontal/vertical)
            - [] Grid layout (organize elements in a grid with a `grid_size_rule` to define how the grid cells are sized)
        - [] Update the style of the elements base on the current theme
        - [] Inputs:
            - [] Elements who are "touchable" will handle mouse inputs based in the priority order.
            - [] Elements with higher z-index will have the priority to handle the inputs first.
            - [] Keyboard inputs will be handled by the focused element.
    -  Render:
        - [] Build the buffer of quad based on the main opengl context. 
        - [x] Clear the screen with the bgui clear color
        - [] Propagate the projection matrix to all elements
        - [] Render all elements base in a z-index order.
        - [] Optimize rendering with techniques like:
            - [] Culling: Skip rendering elements that are outside the viewport.
            - [] Batching: Group similar draw calls to reduce state changes.
*/

#pragma once
#include "elem/element.hpp"
#include "elem/absolute_layout.hpp"
#include "utils/mat.hpp"
#include "utils/vec.hpp"
#include "utils/theme.hpp"
#include <queue>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class bgui {
private:
    butil::theme m_theme;
    std::queue<std::function<void()>> m_gl_calls;
    std::unique_ptr<absolute_layout> m_main_layout;
public:
    bgui(const butil::theme& theme = butil::dark_theme);
    ~bgui();

    static bgui& instance() {
        static bgui instance;
        return instance;
    }

    void init_lib();
    void add_gl_call(const std::function<void()>& f);
    absolute_layout* get_main_layout();

    // \{ 
    // style management
    void set_theme(const butil::theme& gui_theme);
    butil::theme get_theme() const;
    // \}
    // \{
    // rendering
    GLuint get_quad_vao() const;
    void clear() const;

    void update();
    void render();
    void update(layout& lay);
    void render(layout& lay);
    // \}
};
#include "bgui.hpp"
#include <os/os.hpp>
#include <utils/vec.hpp>

bgui::bgui(theme gui_theme)
    : m_theme(gui_theme), m_clear_color({0.0f, 0.0f, 0.0f, 1.0f}) {
    set_theme(gui_theme);
}

bgui::~bgui() {
}

void bgui::set_theme(theme gui_theme) {
    // set the theme and update clear color accordingly
    m_theme = gui_theme;
    if (m_theme == theme::dark) {
        m_clear_color = bgui_utils::color({0.1f, 0.1f, 0.1f, 1.0f});
    } else  if (m_theme == theme::light) {
        m_clear_color = bgui_utils::color({0.9f, 0.9f, 0.9f, 1.0f});
    }
}

theme bgui::get_theme() const {
    return m_theme;
}

GLuint bgui::get_quad_vao() const {
    static GLuint quad_vao = 0;

    static bool initialized = false;
    if (!initialized)
        initialized = true;
    else return quad_vao;
    
    if (quad_vao == 0) {
        float quad_vertices[] = {
            // positions   // texcoords
            0.0f, 1.0f,   0.0f, 1.0f,
            1.0f, 0.0f,   1.0f, 0.0f,
            0.0f, 0.0f,   0.0f, 0.0f,
            0.0f, 1.0f,   0.0f, 1.0f,
            1.0f, 1.0f,   1.0f, 1.0f,
            1.0f, 0.0f,   1.0f, 0.0f
        };
        GLuint VBO;
        glGenVertexArrays(1, &quad_vao);
        glGenBuffers(1, &VBO);
        glBindVertexArray(quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
    return quad_vao;
}

void bgui::clear() const
{
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bgui::update(layout &lay) {
    lay.update();
}

void bgui::render(layout &lay) {
    static std::vector<draw_call> calls;
    calls.clear();
    lay.get_draw_calls(calls);

    for (draw_call& call : calls) {
        call.m_shader.bind();
        glBindVertexArray(call.m_vao);
        glDrawArrays(call.m_mode, 0, call.m_count);
    }
}

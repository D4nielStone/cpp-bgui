#include "bgui.hpp"
#include <os/os.hpp>
#include <utils/vec.hpp>
#include <stdexcept>

static bool init_trigger = false;

bgui::bgui(const butil::theme& gui_theme) : m_main_layout(nullptr), m_theme(gui_theme) {
}

bgui::~bgui() {
}

void bgui::init_lib() {
    init_trigger = true;
    if(!m_main_layout)
        set_layout<absolute_layout>();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    set_theme(m_theme);
}

void bgui::add_gl_call(const std::function<void()> &f) {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    m_gl_calls.push(f);
}

layout *bgui::get_layout() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    return m_main_layout.get();
}

void bgui::set_theme(const butil::theme &gui_theme)
{
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    // set the theme and update clear color accordingly
    m_theme = gui_theme;

    for(auto& elem : m_main_layout->get_elements()) {
        elem->set_theme(m_theme);
    }
}

butil::theme bgui::get_theme() const {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    return m_theme;
}

GLuint bgui::get_quad_vao() const {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
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

void bgui::clear() const {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    glClearColor(m_theme.m_clear_color[0], m_theme.m_clear_color[1], m_theme.m_clear_color[2], m_theme.m_clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    butil::vec2i size = bos::get_window_size();
    glViewport(0, 0, size[0], size[1]);
}

void bgui::update() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    // the main layout must to be resized based on the window size.
    butil::vec2i w_size = bos::get_window_size();
    m_main_layout->set_rect(0.f, 0.f, static_cast<float>(w_size[0]), static_cast<float>(w_size[1]));

    update(*m_main_layout);
}
void bgui::render() {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    render(*m_main_layout);
}

void bgui::update(layout &lay) {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    // call gl functions
    while (!m_gl_calls.empty()) {
        m_gl_calls.front()();
        m_gl_calls.pop();
    }
    
    lay.update();
}

void bgui::render(layout &lay) {
    if(!init_trigger) throw std::runtime_error("BGUI::You must initialize the library.");
    static std::vector<draw_call> calls;
    calls.clear();
    lay.get_draw_calls(calls);

    const butil::mat4 proj = bos::get_projection();

    for (draw_call& call : calls) {
        if(!call.m_material.m_visible) continue;

        call.m_material.set("u_rect", call.m_bounds);
        call.m_material.set("u_uv_min", call.m_uv_min);
        call.m_material.set("u_uv_max", call.m_uv_max);
        call.m_material.set("u_projection", proj);
        call.m_material.bind_uniforms();
        
        glBindVertexArray(call.m_vao);
        glDrawArrays(call.m_mode, 0, call.m_count);
    }
}

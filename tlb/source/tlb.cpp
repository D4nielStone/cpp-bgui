#include <iostream>
#include "tlb.hpp"
#include "bgui.hpp"
#include "elem/text.hpp"
#include "elem/button.hpp"
#include "elem/box.hpp"
#include "elem/linear_layout.hpp"

GLFWwindow* TLB::m_window = nullptr;

void TLB::config_layout() {
    bgui::instance().init_lib();
    auto& lay = bgui::instance().set_layout<linear_layout>();

    // lateral panel
    auto& p = lay.add<linear_layout>();
    p.get_material().m_visible = true;
    p.set_rect(0, 0, 200, 1000);
}

void TLB::error(const std::string &msg) {
    throw std::runtime_error(msg);
}
void TLB::info(const std::string& msg) {
    std::cout << breset << "TLB::" << bgreen << msg << "\n" << breset;
}
void TLB::add_task(const std::string & name) {
    m_tasks.push_back({false, name});
    info("adding task " + name);
}
void TLB::init_graphics() {
    TLB::info("Creating new window...");
    // step 1: init glfw
    if (!glfwInit()) {
        TLB::error("Glfw initialization");
    }
    // step 2: create window and init glad
    
    m_window = glfwCreateWindow(800, 600, "Todo List Bubble v1.0", NULL, NULL);
    if (!m_window) {
        TLB::error("Invalid window");
    };

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLB::error("Glad initialization");
    }

    TLB::info("Window created");

    config_layout();
}
void TLB::window_loop() {
    TLB::info("Starting main loop");
    while(!glfwWindowShouldClose(m_window)) {
        // poll events
        glfwPollEvents();

        // --- //
        // renderingd
        bgui::instance().clear();

        bgui::instance().update();
        bgui::instance().render();
        // --- //

        // swap buffers
        glfwSwapBuffers(m_window);
    }
}
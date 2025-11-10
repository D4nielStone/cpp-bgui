#include <iostream>
#include "tlb.hpp"
#include "bgui.hpp"
#include "elem/text.hpp"
#include "elem/linear_layout.hpp"

GLFWwindow* TLB::m_window = nullptr;

void TLB::config_layout() {
    bgui::instance().init_lib();
    
    auto& lay = bgui::instance().set_layout<linear_layout>(orientation::vertical);
    lay.add<elements::text>("Todo List Bubble", 36);
    lay.add<elements::text>("Look", 36);
    lay.add<elements::text>("Looks fine for me!", 25);
    lay.set_aligniment(alignment::center);
}

void TLB::error(const std::string &msg) {
    throw std::runtime_error(msg);
}
void TLB::info(const std::string& msg) {
    std::cout << breset << "TLB::" << bgreen << msg << "\n" << breset;
}
void TLB::init_graphics() {
    TLB::info("Creating new window...");
    // step 1: init glfw
    if (!glfwInit()) {
        TLB::error("Glfw initialization");
    }
    // step 2: create window and init glad
    /*
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);    
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    */
    
    m_window = glfwCreateWindow(400, 600, "Todo List Bubble Application", NULL, NULL);
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
#include "bgui_backend_vulkan.hpp"
#include <stdexcept>
#include <iostream>
#ifdef BGUI_USE_GLFW
    #include <GLFW/glfw3.h>
#endif

static VkInstance s_vkinstance;
void bkend::set_up_vulkan() {
    bkend::create_vk_instance();
}

void bkend::create_vk_instance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "BGUI Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

#ifdef BGUI_USE_GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

#endif // For now, no validation layers
    createInfo.enabledLayerCount = 0;
    if (vkCreateInstance(&createInfo, nullptr, &s_vkinstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void bkend::shutdown_vulkan() {
    vkDestroyInstance(s_vkinstance, nullptr);
}

void bkend::vulkan_render(butil::draw_data* draw_data) {
}
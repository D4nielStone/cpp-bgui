#pragma once
#include <vulkan/vulkan.h>
#include "utils/draw.hpp"

namespace bkend {
    void vulkan_render(butil::draw_data*);
    void create_vk_instance();
    //void vulkan_clear_texture_cache();
    void set_up_vulkan();
    void shutdown_vulkan();
    //GLuint get_quad_vao();
    //GLuint vulkan_get_texture(const butil::texture& tex);
} // namespace bkend
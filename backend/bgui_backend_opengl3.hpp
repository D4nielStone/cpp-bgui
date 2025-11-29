#pragma once
#include "utils/draw.hpp"

namespace bkend {
    void opengl3_render(butil::draw_data*);
    void set_up_opengl3();
    void shutdown_opengl3();
} // namespace bkend
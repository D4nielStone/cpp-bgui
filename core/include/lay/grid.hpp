#pragma once

#include "layout.hpp"

namespace bgui {

// Uniform cells, filled left to right; columns adapt to the available width.
class grid : public layout {
    vec2i m_cell_size;
    vec2i m_gap;

    int available_width() const;
    int columns(int width) const;

public:
    explicit grid(vec2i cell_size = {160, 40}, vec2i gap = {6, 6});
    void calc_content_size(const layer& lay) override;
    void arrange() override;
};

}

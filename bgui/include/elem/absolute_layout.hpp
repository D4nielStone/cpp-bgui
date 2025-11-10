#pragma once
#include "layout.hpp"

class absolute_layout : public layout {
public:
    absolute_layout() = default;
    ~absolute_layout() = default;

    void update() override {
        for (const auto& elem : get_elements()) {
            elem->update();
        }
    }
    void get_draw_calls(std::vector<draw_call>& calls) override;
};
/**
 * @class Modular Layout
 * @file modular.hpp
 */

#pragma once
#include "layout.hpp"

namespace bgui {
    class modular : public layout {
    public:
        modular(const bgui::orientation& ori = bgui::orientation::horizontal);
        ~modular() = default;
    
        void update() override;
        void fit_to_content() override;
        bgui::layout* as_layout() override { return this; }
    };
}// namespace lay
/**
 * @class Modular Layout
 * @file modular.hpp
 */

#pragma once
#include "layout.hpp"

namespace bgui {
    class modular : public layout {
    private:
        orientation m_orientation;
    public:
        modular(const bgui::orientation& ori = bgui::orientation::horizontal);
        ~modular() = default;
    
        void on_update() override;
        void set_orientation(const orientation& ori) { m_orientation = ori; };
        bgui::layout* as_layout() override { return this; }
    };
}// namespace lay
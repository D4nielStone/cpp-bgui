/**
 * @class Layout
 * @file layout.hpp
 * @brief A base layout class that can contain multiple child elements and manage their arrangement.
 */

#pragma once
#include "elem/element.hpp"
#include <algorithm>
#include <queue>

namespace bgui {
    class layout : public element {
    protected:
        std::vector<std::unique_ptr<element>> m_elements;
    public:
        layout();
        ~layout() = default;
    
        virtual void apply_theme(const bgui::theme& theme) {
            m_material.set("bg_color", theme.m_box_color);
            m_material.set("bordered", true);
            m_material.set("border_radius", 4.f);
            m_material.set("border_size", 1.f);
            m_material.set("border_color", theme.m_button_border_color);
            for(auto& e: m_elements) e->apply_theme(theme);
        };
    
        template<typename T, typename... Args>
        T& add(Args&&... args) {
            auto elem = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *elem;
            m_elements.push_back(std::move(elem));
            return ref;
        }
    
        bool remove(element* elem) {
            auto it = std::remove_if(m_elements.begin(), m_elements.end(),
                [elem](const std::unique_ptr<element>& e) { return e.get() == elem; });
            if (it != m_elements.end()) {
                m_elements.erase(it, m_elements.end());
                return true;
            }
            return false;
        }
        void update() override;
        void get_requests(bgui::draw_data* calls);
        virtual void fit_to_content();
        std::vector<std::unique_ptr<element>>& get_elements();
        bgui::layout* as_layout() override { return this; }
    };
} // namespace bgui
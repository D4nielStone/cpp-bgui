/**
 * @class Layout
 * @file layout.hpp
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
    
        void apply_style(const bgui::style& style) override {
        element::apply_style(style);
            for(auto& e: m_elements) e->apply_style(style);
        };
    
        template<typename T, typename... Args>
        T& add(Args&&... args) {
            auto elem = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *elem;
            ref.set_parent(this);
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
        void on_update() override;
        void get_requests(bgui::draw_data* calls);
        std::vector<std::unique_ptr<element>>& get_elements();
        bgui::layout* as_layout() override { return this; }
    };
} // namespace bgui
#pragma once
#include "element.hpp"
#include <algorithm>
#include <queue>

class layout : public element {
protected:
    std::queue<std::unique_ptr<layout>> m_modals;
    std::vector<std::unique_ptr<element>> m_elements;
public:
    layout();
    ~layout() = default;

    virtual void apply_theme(const butil::theme& theme) {
        m_material.set("u_bg_color", theme.m_box_color);
        m_material.set("u_bordered", true);
        m_material.set("u_border_radius", 4.f);
        m_material.set("u_border_size", 1.f);
        m_material.set("u_border_color", theme.m_button_border_color);
        for(auto& e: m_elements) e->apply_theme(theme);
    };

    template<typename T, typename... Args>
    T& add(Args&&... args) {
        auto elem = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *elem;
        m_elements.push_back(std::move(elem));
        return ref;
    }

    template<typename T, typename... Args>
    T& new_modal(Args&&... args) {
        auto m = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *m;
        m_modals.push(std::move(m));
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
    void get_draw_requests(std::vector<butil::draw_request>& calls);
    virtual void fit_to_content();
    std::queue<std::unique_ptr<layout>> &get_modals();
    std::vector<std::unique_ptr<element>>& get_elements();
    void pop_modal();
    layout* as_layout() override { return this; }
};
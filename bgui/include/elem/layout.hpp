#pragma once
#include "element.hpp"
#include <algorithm>

enum class orientation {
    vertical,
    horizontal
};
enum class alignment {
    start,
    center,
    end,
    stretch
};
class layout : public element {
protected:
    std::vector<std::unique_ptr<element>> m_elements;
    orientation m_orientation;
    alignment m_alignment, m_cross_alignment;
    butil::vec<2, unsigned int> m_spacing_elements;
public:
    layout();
    ~layout() = default;

    virtual void set_theme(const butil::theme& theme) {
        m_material.set("u_bg_color", theme.m_box_color);
        m_material.set("u_bordered", true);
        m_material.set("u_border_radius", 5.f);
        m_material.set("u_border_size", 5.f);
        m_material.set("u_border_color", theme.m_box_color);
    };

    template<typename T, typename... Args>
    T& add(Args&&... args) {
        auto elem = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *elem;
        m_elements.push_back(std::move(elem));
        return ref;
    }

    const std::vector<std::unique_ptr<element>>& get_elements() const {
        return m_elements;
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
    void set_cross_aligniment(const alignment& al) {
        m_cross_alignment = al;
    }
    void set_aligniment(const alignment& al) {
        m_alignment = al;
    }

    void set_orientation(const orientation& o) {
        m_orientation = o;
    }

    void set_spacing_elements(const unsigned int a, const unsigned int b) {
        m_spacing_elements = {a, b};
    }
};
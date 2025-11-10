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
    alignment m_alignment;
    butil::vec<2, unsigned int> m_spacing;
public:
    layout() : m_orientation(orientation::horizontal), m_alignment(alignment::start), m_spacing(1) {
        m_material.m_visible = false;
    };
    ~layout() = default;

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
    void set_aligniment(const alignment& al) {
        m_alignment = al;
    }

    void set_orientation(const orientation& o) {
        m_orientation = o;
    }

    void set_spacing(const unsigned int a, const unsigned int b) {
        m_spacing = {a, b};
    }
};
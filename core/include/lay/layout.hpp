#pragma once

#include "elem/element.hpp"
#include "lay/layer.hpp"
#include "os/style_manager.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace bgui {

class layout;

template<typename T>
class scoped_element {
    friend class layout;

    layout* m_parent{};
    T* m_element{};

    scoped_element(layout* parent, T* element) noexcept
        : m_parent(parent),
          m_element(element) {}

public:
    scoped_element() noexcept = default;

    scoped_element(const scoped_element&) = delete;
    scoped_element& operator=(const scoped_element&) = delete;

    scoped_element(scoped_element&& other) noexcept
        : m_parent(std::exchange(other.m_parent, nullptr)),
          m_element(std::exchange(other.m_element, nullptr)) {}

    scoped_element& operator=(scoped_element&& other) noexcept {
        if (this == &other)
            return *this;

        reset();

        m_parent = std::exchange(other.m_parent, nullptr);
        m_element = std::exchange(other.m_element, nullptr);

        return *this;
    }

    ~scoped_element() noexcept {
        reset();
    }

    void reset() noexcept {
        if (!m_parent || !m_element)
            return;

        m_parent->remove(m_element);

        m_parent = nullptr;
        m_element = nullptr;
    }

    T& get() const noexcept {
        return *m_element;
    }

    T& operator*() const noexcept {
        return *m_element;
    }

    T* operator->() const noexcept {
        return m_element;
    }

    explicit operator bool() const noexcept {
        return m_element != nullptr;
    }

    T* release() noexcept {
        T* element = m_element;

        m_parent = nullptr;
        m_element = nullptr;

        return element;
    }
};

class layout : public element {
protected:
    std::map<layer, std::vector<std::unique_ptr<element>>> m_elements;
    bool m_resizable{false};

private:
    template<typename T, layer Lay, typename... Args>
    T& insert_element(Args&&... args) {
        auto elem = std::make_unique<T>(
            std::forward<Args>(args)...
        );

        T& ref = *elem;

        ref.set_parent(this);

        m_elements[Lay].push_back(std::move(elem));

        return ref;
    }

public:
    layout();
    ~layout() override = default;

    template<typename T, layer Lay = layer::base, typename... Args>
    scoped_element<T> add(Args&&... args) {
        return scoped_element<T>(
            this,
            &insert_element<T, Lay>(
                std::forward<Args>(args)...
            )
        );
    }

    template<typename T, layer Lay = layer::base, typename... Args>
    T& add_persistent(Args&&... args) {
        return insert_element<T, Lay>(
            std::forward<Args>(args)...
        );
    }

    bool remove(element* elem) {
        if (!elem)
            return false;

        for (auto& [lay, elems] : m_elements) {
            auto it = std::find_if(
                elems.begin(),
                elems.end(),
                [elem](const std::unique_ptr<element>& current) {
                    return current.get() == elem;
                }
            );

            if (it == elems.end())
                continue;

            elems.erase(it);
            return true;
        }

        return false;
    }

    void mark_children_style_dirty() {
        for (auto& [lay, elems] : m_elements) {
            for (auto& elem : elems) {
                if (!elem || !elem->is_enabled())
                    continue;

                elem->mark_style_dirty();

                if (auto* child = elem->as_layout())
                    child->mark_children_style_dirty();
            }
        }
    }

    void cascade_style() {
        auto& sm = style_manager::get_instance();

        if (sm.has_theme_changed()) {
            mark_style_dirty();
            sm.clear_theme_changed_flag();
        }

        compute_style();

        for (auto& [lay, elems] : m_elements) {
            for (auto& elem : elems) {
                if (!elem || !elem->is_enabled())
                    continue;

                if (auto* child = elem->as_layout()) {
                    child->cascade_style();
                } else if (elem->is_style_dirty()) {
                    elem->compute_style();
                }
            }
        }
    }

    void on_update() override;

    void get_requires(bgui::draw_data* calls);

    std::map<layer, std::vector<std::unique_ptr<element>>>& get_elements();

    void set_resizable(bool enabled);

    bool is_resizable() const {
        return m_resizable;
    }

    std::vector<element*> get_elements_by_class(
        const std::string& cls
    ) {
        std::vector<element*> result;

        for (auto& [lay, elems] : m_elements) {
            for (auto& elem : elems) {
                if (!elem)
                    continue;

                if (elem->has_class(cls))
                    result.push_back(elem.get());
            }
        }

        return result;
    }

    bgui::layout* as_layout() override {
        return this;
    }
};

}
/**
 * @class Layout
 * @file layout.hpp
 */

#pragma once
#include "elem/element.hpp"
#include "lay/layer.hpp"
#include "os/style_manager.hpp"
#include <algorithm>
#include <queue>
#include <map>
#include <utility>

namespace bgui {
    class layout;

    template<typename T>
    class scoped_element {
        friend class layout;
        layout* m_parent;
        T* m_element;

        scoped_element(layout* parent, T* elem) noexcept : m_parent(parent), m_element(elem) {}
    public:
        scoped_element(const scoped_element&) = delete;
        scoped_element& operator=(const scoped_element&) = delete;
        scoped_element(scoped_element&& other) noexcept
            : m_parent(std::exchange(other.m_parent, nullptr)),
              m_element(std::exchange(other.m_element, nullptr)) {}
        scoped_element& operator=(scoped_element&& other) noexcept;
        ~scoped_element() noexcept;

        T& get() const noexcept { return *m_element; }
        T& operator*() const noexcept { return *m_element; }
        T* operator->() const noexcept { return m_element; }
        explicit operator bool() const noexcept { return m_element != nullptr; }
    };

    class layout : public element {
    protected:
        std::map<bgui::layer, std::vector<std::unique_ptr<element>>> m_elements;
        bool m_resizable{false};
    private:
        template<typename T, layer lay, typename... Args>
        T& insert_element(Args&&... args) {
            auto elem = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *elem;
            ref.set_parent(this);
            m_elements[lay].push_back(std::move(elem));
            return ref;
        }
    public:
        layout();
        ~layout() = default;
    
        void mark_children_style_dirty() {
            for(auto& [lay, elems] : get_elements()) {
                for(auto& elem : elems) {
                    if(!elem->is_enabled()) continue;
                    elem->mark_style_dirty();
                    
                    if (auto* lay = elem->as_layout()) {
                        lay->mark_children_style_dirty();
                    }
                }
            }
        }
        void cascade_style() {
            auto& sm = style_manager::get_instance();
            
            // If theme changed, mark all styles as dirty
            if (sm.has_theme_changed()) {
                mark_style_dirty();
                sm.clear_theme_changed_flag();
            }
            
            compute_style();
            // compute children style
            for (auto& [lay, elems] : get_elements()) {
                for(auto& elem : elems) {
                    if(!elem->is_enabled()) continue;
                    if (elem->as_layout()) {
                        elem->as_layout()->cascade_style();
                    } else if (elem->is_style_dirty()) {
                        elem->compute_style();
                    }
                }
            }
        }
        template<typename T, layer lay = layer::base, typename... Args>
        scoped_element<T> add(Args&&... args) {
            return scoped_element<T>(this, &insert_element<T, lay>(std::forward<Args>(args)...));
        }

        template<typename T, layer lay = layer::base, typename... Args>
        T& add_persistent(Args&&... args) {
            return insert_element<T, lay>(std::forward<Args>(args)...);
        }
    
        bool remove(element* elem) {
            for(auto& [lay, elems] : m_elements) {
                auto it = std::remove_if(elems.begin(), elems.end(),
                    [elem](const std::unique_ptr<element>& e) { return e.get() == elem; });
                if (it != elems.end()) {
                    elems.erase(it, elems.end());
                    return true;
                }
            }
            return false;
        }
        void on_update() override;
        void get_requires(bgui::draw_data* calls);
        std::map<layer, std::vector<std::unique_ptr<element>>>& get_elements();
        void set_resizable(bool enabled);
        bool is_resizable() const { return m_resizable; }
    
        std::vector<element*> get_elements_by_class(const std::string& cls) {
            for(auto& [lay, elems] : get_elements()) {
                std::vector<element*> result;
                for(auto& elem : elems) {
                    if(!elem) continue;
                    if(elem->has_class(cls)) {
                        result.push_back(elem.get());
                    }
                }
                return result;
            }
            return {};
        }
        bgui::layout* as_layout() override { return this; }
    };

    template<typename T>
    scoped_element<T>& scoped_element<T>::operator=(scoped_element&& other) noexcept {
        if (this == &other) return *this;
        if (m_parent && m_element) m_parent->remove(m_element);
        m_parent = std::exchange(other.m_parent, nullptr);
        m_element = std::exchange(other.m_element, nullptr);
        return *this;
    }

    template<typename T>
    scoped_element<T>::~scoped_element() noexcept {
        if (m_parent && m_element) m_parent->remove(m_element);
    }
} // namespace bgui
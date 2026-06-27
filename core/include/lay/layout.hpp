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

namespace bgui {
    class layout : public element {
    protected:
        std::map<bgui::layer, std::vector<std::unique_ptr<element>>> m_elements;
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
                        if (elem->is_style_dirty()) {
                            if(elem->as_layout()) {
                                elem->as_layout()->cascade_style();
                            } else
                        elem->compute_style();
                    }
                }
            }
        }
        template<typename T, layer lay = layer::base, typename... Args>
        T& add(Args&&... args) {
            auto elem = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *elem;
            ref.set_parent(this);
            m_elements[lay].push_back(std::move(elem));
            return ref;
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
} // namespace bgui
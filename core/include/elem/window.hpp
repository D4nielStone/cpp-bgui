#pragma once
#include "lay/linear.hpp"
#include "elem/image.hpp"
#include "elem/inputbox.hpp"
#include "elem/text.hpp"
#include "utils/syntax_highlight.hpp"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace bgui {
    class button;

    class window : public linear {
    private:
        text* m_title{nullptr};
        linear* m_header{nullptr};
        linear* m_context{nullptr};
        image* m_icon{nullptr};
        button* m_close_button{nullptr};
        float m_icon_aspect_ratio = 1.f;
        int m_icon_height = 0;
        std::shared_ptr<const std::vector<syntax_highlight_rule>> m_highlight_rules;
        vec2i m_pinned_drag_distance{0, 0};
        bool m_floating = true;
        bool m_dragging = false;

        void initialize_context();
    public:
        window();
        window(const char* title, bool floating = true);
        ~window()=default;

        template<typename T, layer Lay = layer::base, typename... Args>
        scoped_element<T> add(Args&&... args) {
            auto added = m_context->add<T, Lay>(std::forward<Args>(args)...);
            if constexpr (std::is_base_of_v<inputbox, T>) {
                if (m_highlight_rules)
                    added->set_highlight_rules(m_highlight_rules);
            }
            return added;
        }

        linear& get_context() { return *m_context; }

        void on_update() override;
        void set_icon(const std::string& path);
        void use_highlight_config(const std::string& path);
        void set_title(const std::string& title) { m_title->set_buffer(title); }
        text& get_title() { return *m_title; }
        bool clips_children() const override { return true; }
        bool is_floating() const { return m_floating; }
        bool is_dragging() const { return m_dragging; }
        void set_floating(bool floating);
        void set_tabbed(bool tabbed);
    };
}// namespace bgui
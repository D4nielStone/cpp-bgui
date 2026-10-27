#pragma once

#include "elem/button.hpp"
#include "elem/icon.hpp"

#include <string_view>

namespace bgui {
    class tree : public linear {
    public:
        explicit tree(
            const std::string& label,
            float scale = 0.3,
            bool expanded = false
        );

        tree& add_child(const std::string& label);
        void set_icon(std::string_view name, float scale = 0.4f);
        bool has_icon() const noexcept { return m_icon != nullptr; }
        void set_expanded(bool expanded);
        bool is_expanded() const noexcept { return m_expanded; }
        linear& children() noexcept { return *m_children; }
        /** The row can select an item while the disclosure still expands it. */
        button& header() noexcept { return *m_label_button; }
        text& label() noexcept { return *m_label_text; }
        button& disclosure() noexcept { return *m_disclosure; }
        /** Leaves keep the same indentation without a clickable disclosure. */
        void set_leaf(bool leaf);
        void set_on_drop(std::function<void(const vec2i&)> callback);

    private:
        class header_button;
        float m_scale;
        button* m_label_button{nullptr};
        text* m_label_text{nullptr};
        button* m_disclosure{nullptr};
        icon* m_disclosure_icon{nullptr};
        linear* m_icon_slot{nullptr};
        icon* m_icon{nullptr};
        linear* m_children{nullptr};
        bool m_expanded{false};
        bool m_leaf{false};
        std::function<void(const vec2i&)> m_on_drop;
    };
}

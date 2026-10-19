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

    private:
        float m_scale;
        button* m_label_button{nullptr};
        text* m_disclosure{nullptr};
        linear* m_icon_slot{nullptr};
        icon* m_icon{nullptr};
        linear* m_children{nullptr};
        bool m_expanded{false};
    };
}

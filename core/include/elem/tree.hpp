#pragma once

#include "elem/button.hpp"

namespace bgui {
    class tree : public linear {
    public:
        explicit tree(
            const std::string& label,
            float scale = 0.4f,
            bool expanded = false
        );

        tree& add_child(const std::string& label);
        void set_expanded(bool expanded);
        bool is_expanded() const noexcept { return m_expanded; }
        linear& children() noexcept { return *m_children; }

    private:
        std::string m_label;
        float m_scale;
        button* m_label_button{nullptr};
        linear* m_children{nullptr};
        bool m_expanded{false};
    };
}

#include "elem/tree.hpp"

namespace bgui {
    tree::tree(
        const std::string& label,
        const float scale,
        const bool expanded
    ) : linear(orientation::vertical),
        m_label(label),
        m_scale(scale) {
        type = "tree";
        style.layout.require_mode(mode::match_parent, mode::wrap_content);

        m_label_button = &add<button>("", scale, [this]() {
            set_expanded(!m_expanded);
        });
        m_label_button->style.layout.require_mode(mode::match_parent, mode::wrap_content);
        m_label_button->add_class("tree-element-label");
        m_label_button->get_label().add_class("tree-element-label-text");

        m_children = &add<linear>(orientation::vertical);
        m_children->style.layout.require_mode(mode::match_parent, mode::wrap_content);
        m_children->style.layout.set_padding(12, 0);
        m_children->add_class("tree-element-children");

        set_expanded(expanded);
    }

    tree& tree::add_child(const std::string& label) {
        auto& child = m_children->add<tree>(label, m_scale);
        child.style.layout.require_mode(mode::match_parent, mode::wrap_content);
        return child;
    }

    void tree::set_expanded(const bool expanded) {
        m_expanded = expanded;
        m_children->set_enable(expanded);
        m_label_button->get_label().set_buffer(
            std::string(expanded ? "▾ " : "▸ ") + m_label
        );
    }
}

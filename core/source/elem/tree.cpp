#include "elem/tree.hpp"
#include "os/os.hpp"
#include <cstdlib>

namespace bgui {
    class tree::header_button final : public button {
    public:
        header_button(tree& owner, float scale)
            : button("", scale, [&owner]() {
                owner.set_expanded(!owner.is_expanded());
            }), m_owner(owner) {}

        void on_pressed() override {
            m_drag_distance = 0;
            button::on_pressed();
        }
        void set_drag(const vec2i& delta) override {
            button::set_drag(delta);
            if (m_owner.m_on_drop)
                m_drag_distance += std::abs(delta.x) + std::abs(delta.y);
        }
        void on_released() override {
            if (m_drag_distance < 5 || !m_owner.m_on_drop) {
                button::on_released();
                return;
            }
            element::on_released();
            m_owner.m_on_drop(get_mouse_position());
        }
    private:
        tree& m_owner;
        int m_drag_distance{0};
    };

    tree::tree(
        const std::string& label,
        const float scale,
        const bool expanded
    ) : linear(orientation::vertical),
        m_scale(scale) {
        type = "tree";
        style.layout.require_mode(mode::match_parent, mode::wrap_content);

        m_label_button = &add<header_button>(*this, scale);
        m_label_button->style.layout.require_mode(
            mode::match_parent,
            mode::wrap_content
        );
        m_label_button->style.layout.align = vec<2, alignment>{
            alignment::start,
            alignment::center
        };
        m_label_button->add_class("tree-element-label");
        m_label_button->get_label().set_enable(false);

        m_disclosure = &m_label_button->add<button>("", scale, [this]() {
            set_expanded(!m_expanded);
        });
        m_disclosure->add_class("tree-disclosure");
        m_disclosure->style.layout.require_size(14.f, 18.f);
        m_disclosure->style.layout.padding = vec4i{0};
        m_disclosure->style.layout.align = vec<2, alignment>{alignment::center, alignment::center};
        m_disclosure->get_label().set_enable(false);
        m_disclosure_icon = &m_disclosure->add<icon>("chevron_right", scale * 0.7f);
        m_disclosure_icon->receives_input(false);

        m_icon_slot = &m_label_button->add<linear>(orientation::horizontal);
        m_icon_slot->style.layout.require_mode(
            mode::pixel,
            mode::wrap_content
        );
        m_icon_slot->style.layout.require_width(mode::pixel, 0.f);
        m_icon_slot->style.layout.limit_min = vec2i{0, 0};

        auto& label_text = m_label_button->add<text>(label, scale);
        m_label_text = &label_text;
        m_icon_slot->receives_input(false);
        label_text.receives_input(false);
        label_text.add_class("tree-element-label-text");
        label_text.style.layout.require_mode(
            mode::stretch,
            mode::wrap_content
        );
        label_text.style.layout.align = vec<2, alignment>{
            alignment::start,
            alignment::center
        };

        m_children = &add<linear>(orientation::vertical);
        m_children->style.layout.require_mode(mode::match_parent, mode::wrap_content);
        m_children->style.layout.padding = vec4i{18, 0, 0, 0};
        m_children->add_class("tree-element-children");

        set_expanded(expanded);
    }

    tree& tree::add_child(const std::string& label) {
        auto& child = m_children->add<tree>(label, m_scale);
        child.style.layout.require_mode(mode::match_parent, mode::wrap_content);
        return child;
    }

    void tree::set_icon(const std::string_view name, const float scale) {
        for (auto& [layer, elements] : m_icon_slot->get_elements()) {
            (void)layer;
            while (!elements.empty())
                m_icon_slot->remove(elements.back().get());
        }
        m_icon = nullptr;

        if (name.empty()) {
            m_icon_slot->style.layout.require_width(mode::pixel, 0.f);
            return;
        }

        m_icon = &m_icon_slot->add<icon>(name, scale);
        m_icon->receives_input(false);
        m_icon->style.layout.require_mode(
            mode::wrap_content,
            mode::wrap_content
        );
        m_icon_slot->style.layout.require_width(mode::pixel, 20.f);
    }

    void tree::set_expanded(const bool expanded) {
        m_expanded = expanded && !m_leaf;
        m_children->set_enable(m_expanded);
        m_disclosure_icon->set_buffer(m_expanded ? "\xef\x81\xb8" : "\xef\x81\x94");
    }

    void tree::set_leaf(const bool leaf) {
        m_leaf = leaf;
        m_disclosure->receives_input(!leaf);
        m_disclosure_icon->set_enable(!leaf);
        if (leaf)
            set_expanded(false);
    }

    void tree::set_on_drop(std::function<void(const vec2i&)> callback) {
        m_on_drop = std::move(callback);
    }
}

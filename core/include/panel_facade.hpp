#pragma once

#include "elem/button.hpp"
#include "elem/checkbox.hpp"
#include "elem/field.hpp"
#include "elem/window.hpp"
#include "lay/dock.hpp"

#include <functional>
#include <string>
#include <utility>

namespace bgui {

// Borrowed builder: the layout owns every widget and must outlive this view.
class panel_content {
public:
    explicit panel_content(linear& parent) noexcept : m_parent(parent) {}
    explicit panel_content(window& parent) noexcept : m_parent(parent.get_context()) {}

    template<typename T, layer Layer = layer::base, typename... Args>
    T& add(Args&&... args) const {
        return m_parent.add<T, Layer>(std::forward<Args>(args)...);
    }

    panel_content row() const { return group(orientation::horizontal); }
    panel_content column() const { return group(orientation::vertical); }

    text& label(const std::string& value, float scale = .3f) const {
        return add<text>(value, scale);
    }

    button& action(const std::string& title, std::function<void()> callback,
        float scale = .3f) const {
        return add<button>(title, scale, std::move(callback));
    }

    field& property(const std::string& title, const std::string& value,
        std::function<void(const std::string)> callback = {}, float scale = .3f,
        input_mode input_kind = input_mode::inputbox) const {
        auto& result = add<field>(title, value, scale, std::move(callback), "", input_kind);
        result.style.layout.require_mode(mode::match_parent, mode::wrap_content);
        result.get_inputbox().style.layout.require_width(mode::stretch);
        if (input_kind != input_mode::multiline) {
            result.get_inputbox().style.layout.require_height(mode::pixel, 26.f);
            result.get_label().set_wrap(false);
        }
        return result;
    }

    checkbox& toggle(const std::string& title, bool value,
        std::function<void(bool)> callback = {}, float scale = .3f) const {
        auto& result = add<checkbox>(title, scale, value);
        result.set_on_change(callback);
        return result;
    }

    linear& native() const noexcept { return m_parent; }

private:
    panel_content group(orientation direction) const {
        auto& result = add<linear>(direction);
        result.style.layout.require_mode(mode::match_parent, mode::wrap_content);
        return panel_content(result);
    }

    linear& m_parent;
};

// Creating/destroying a facade never transfers ownership away from BGUI.
class panel_facade {
public:
    explicit panel_facade(window& target) : m_window(target),
        m_body(target.add<linear>(orientation::vertical)) {
        m_body.style.layout.require_mode(mode::match_parent, mode::match_parent);
        m_body.set_scrollable(true);
    }

    panel_facade(dock& parent, const std::string& title,
        dock_area area = dock_area::center, float scale = .3f)
        : panel_facade(parent.add_window(title, area, scale)) {}

    panel_content content() const noexcept { return panel_content(m_body); }
    window& native() const noexcept { return m_window; }

private:
    window& m_window;
    linear& m_body;
};

} // namespace bgui

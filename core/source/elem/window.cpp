#include "elem/window.hpp"
#include "elem/button.hpp"
#include "lay/dock.hpp"
#include "bgui.hpp"
#include "os/asset_manager.hpp"
#include "os/os.hpp"

#include <algorithm>
#include <memory>
#include <vector>

bgui::window::window() : linear(bgui::orientation::vertical) {
    type = "window";
    initialize_context();
}

bgui::window::window(const char* title, float font_scale, bool floating)
    : linear(bgui::orientation::vertical),
      m_title(nullptr),
      m_header(nullptr),
      m_icon(nullptr) {
    type = "window";

    set_position(20, 20);

    m_header = &add<bgui::linear>(bgui::orientation::horizontal);
    m_header->add_class("window-header");
    m_header->style.layout.require_mode(
        bgui::mode::stretch,
        bgui::mode::wrap_content
    );

    m_icon = &m_header->add<bgui::image>();
    m_icon->set_size(1.f, 1.f);

    m_title = &m_header->add<bgui::text>(title, font_scale);
    m_title->add_class("window-label");
    m_title->style.layout.require_mode(
        bgui::mode::stretch,
        bgui::mode::wrap_content
    );

    auto& controls = m_header->add<bgui::linear>(
        bgui::orientation::horizontal
    );

    controls.add_class("window-controls");
    controls.style.layout.require_mode(
        bgui::mode::wrap_content,
        bgui::mode::stretch
    );

    m_close_button = &controls.add<bgui::traffic_light>(
        bgui::traffic_light_type::close,
        [this]() {
            auto* parent = get_parent();

            bgui::add_function([this, parent]() {
                if (auto* dock_parent = dynamic_cast<bgui::dock*>(parent)) {
                    dock_parent->remove_window(this);
                } else if (parent) {
                    parent->remove(this);
                }
            });
        }
    );
    m_close_button->add_class("window-close-button");

    m_minimize_button = &controls.add<bgui::traffic_light>(
        bgui::traffic_light_type::minimize,
        [this]() {
            minimize();
        }
    );
    m_minimize_button->add_class("window-minimize-button");

    m_maximize_button = &controls.add<bgui::traffic_light>(
        bgui::traffic_light_type::maximize,
        [this]() {
            maximize();
        }
    );
    m_maximize_button->add_class("window-maximize-button");

    initialize_context();
    set_floating(floating);
}

void bgui::window::initialize_context() {
    m_context = &add<bgui::linear>(bgui::orientation::vertical);
    m_context->set_scrollable(true);
    m_context->style.layout.require_mode(
        bgui::mode::match_parent,
        bgui::mode::match_parent
    );
    m_context->style.visual.visible = false;
}

void bgui::window::on_update() {
    if (is_floating()) {
        const auto drag = m_title->is_drag();

        if (drag[0] || drag[1]) {
            if (m_maximized && m_restore_rect.z > 0 && m_restore_rect.w > 0) {
                m_maximized = false;
                set_floating_rect(m_restore_rect);
            }
            set_position(
                processed_x() + drag[0],
                processed_y() + drag[1]
            );
            if (m_minimized) {
                m_minimized_rect.x += drag[0];
                m_minimized_rect.y += drag[1];
            }
            m_dragging = true;
        } else if (!bgui::get_pressed(bgui::input_key::mouse_left)) {
            m_dragging = false;
        }
    } else {
        const auto drag = m_title->is_drag();

        if (bgui::get_pressed(bgui::input_key::mouse_left) &&
            (drag.x != 0 || drag.y != 0)) {
            m_pinned_drag_distance.x += drag.x;
            m_pinned_drag_distance.y += drag.y;

            const int threshold = std::max(
                12,
                static_cast<int>(24.f * bgui::get_global_scale())
            );

            if (m_pinned_drag_distance.x * m_pinned_drag_distance.x +
                m_pinned_drag_distance.y * m_pinned_drag_distance.y >=
                    threshold * threshold) {
                set_floating(true);

                set_position(
                    processed_x() + m_pinned_drag_distance.x,
                    processed_y() + m_pinned_drag_distance.y
                );

                m_dragging = true;
                m_pinned_drag_distance = {0, 0};
            }
        } else if (!bgui::get_pressed(bgui::input_key::mouse_left)) {
            m_pinned_drag_distance = {0, 0};
            m_dragging = false;
        }
    }

    m_title->set_drag({0, 0});
    linear::on_update();

    // The icon occupies the header's content box. Including padding here
    // feeds it back into wrap_content and grows the header every frame.
    const auto& padding = m_header->computed_style.layout.padding;
    const auto& margin = m_icon->computed_style.layout.margin;
    const int header_height = std::max(
        0, m_header->processed_height() - padding.y - padding.w -
            margin.y - margin.w);
    const float scale = bgui::get_global_scale();

    if (header_height > 0 &&
        scale > 0.f &&
        header_height != m_icon_height) {
        const int icon_width = std::max(
            1,
            static_cast<int>(header_height * m_icon_aspect_ratio)
        );

        m_icon->set_size(icon_width / scale, header_height / scale);
        m_icon_height = header_height;
    }
}

void bgui::window::set_icon(const std::string& path) {
    const auto& texture =
        bgui::asset_manager::get_instance().load_texture(path);

    m_icon->set_texture(texture);

    if (texture.m_size.y > 0.f) {
        m_icon_aspect_ratio = texture.m_size.x / texture.m_size.y;
    }

    m_icon_height = 0;
}

void bgui::window::use_highlight_config(const std::string& path) {
    auto rules = std::make_shared<const std::vector<syntax_highlight_rule>>(
        load_syntax_highlight_config(path)
    );

    m_highlight_rules = std::move(rules);

    for (auto& [lay, elements] : m_context->get_elements()) {
        for (auto& element : elements) {
            if (auto* input = dynamic_cast<inputbox*>(element.get())) {
                input->set_highlight_rules(m_highlight_rules);
            }
        }
    }
}

void bgui::window::set_floating_rect(const vec4i& rect) {
    const float scale = bgui::get_global_scale();
    if (scale <= 0.f || rect.z <= 0 || rect.w <= 0)
        return;

    set_position(rect.x, rect.y);
    style.layout.require_mode(bgui::mode::pixel, bgui::mode::pixel);
    style.layout.require_size(rect.z / scale, rect.w / scale);
    mark_style_dirty();
}

void bgui::window::minimize() {
    if (!m_floating || !m_context || !m_header)
        return;

    if (m_minimized) {
        m_minimized = false;
        m_context->set_enable(true);
        set_floating_rect(m_minimized_rect);
        return;
    }

    const auto rect = processed_rect();
    const int header_height = m_header->processed_height();
    if (rect.z <= 0 || rect.w <= 0 || header_height <= 0)
        return;

    m_minimized_rect = rect;
    m_minimized = true;
    m_context->set_enable(false);
    set_floating_rect({rect.x, rect.y, rect.z, header_height});
}

void bgui::window::maximize() {
    if (!m_floating || !get_parent())
        return;

    const auto current_rect = m_minimized ? m_minimized_rect : processed_rect();
    if (current_rect.z <= 0 || current_rect.w <= 0)
        return;

    if (m_minimized) {
        m_minimized = false;
        m_context->set_enable(true);
    }

    if (m_maximized) {
        m_maximized = false;
        set_floating_rect(m_restore_rect);
        return;
    }

    const auto parent_rect = get_parent()->processed_rect();
    if (parent_rect.z <= 0 || parent_rect.w <= 0)
        return;

    m_restore_rect = current_rect;
    m_maximized = true;
    set_floating_rect(parent_rect);
}

void bgui::window::set_floating(bool floating) {
    if (floating && !m_floating) {
        const float scale = bgui::get_global_scale();

        if (scale > 0.f &&
            processed_width() > 0 &&
            processed_height() > 0) {
            style.layout.require_mode(
                bgui::mode::pixel,
                bgui::mode::pixel
            );

            style.layout.require_size(
                processed_width() / scale,
                processed_height() / scale
            );

            mark_style_dirty();
        }
    }

    m_floating = floating;
    if (!floating) {
        m_minimized = false;
        m_maximized = false;
        if (m_context)
            m_context->set_enable(true);
    }
    set_flex(!floating);

    if (m_close_button) {
        m_close_button->set_enable(floating);
    }

    if (m_minimize_button) {
        m_minimize_button->set_enable(floating);
    }

    if (m_maximize_button) {
        m_maximize_button->set_enable(floating);
    }

    set_resizable(floating);
}

void bgui::window::set_tabbed(bool tabbed) {
    if (m_header) {
        m_header->set_enable(!tabbed);
    }
}
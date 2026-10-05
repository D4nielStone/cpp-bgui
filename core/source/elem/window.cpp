#include "elem/window.hpp"
#include "elem/button.hpp"
#include "lay/dock.hpp"
#include "bgui.hpp"
#include "os/asset_manager.hpp"
#include "os/os.hpp"
#include <algorithm>

bgui::window::window() : linear(bgui::orientation::vertical) {
    type = "window";
    initialize_context();
}

bgui::window::window(const char* title, bool floating) : linear(bgui::orientation::vertical), m_title(nullptr), m_header(nullptr), m_icon(nullptr) {
    type = "window";
    // window widget experiment
    //TODO:: add parse init config for window
    set_position(20, 20);

    // testing the header:
    m_header = &add_persistent<bgui::linear>(bgui::orientation::horizontal);
    m_header->add_class("window-header");
    m_header->style.layout.require_mode(bgui::mode::stretch, bgui::mode::wrap_content);
    m_icon = &m_header->add_persistent<bgui::image>();
    m_icon->set_size(1.f, 1.f);
    set_icon("bubble.png");
    m_title = &m_header->add_persistent<bgui::text>(title, 0.4);
    m_title->add_class("window-label");
    m_title->style.layout.require_mode(bgui::mode::stretch, bgui::mode::wrap_content);
    // TODO: switch to image button later
    m_close_button = &m_header->add_persistent<bgui::button>(" X ", 0.4f, [this](){
        auto* parent = get_parent();
        bgui::add_function([this, parent]() {
            if (auto* dock_parent = dynamic_cast<bgui::dock*>(parent))
                dock_parent->remove_window(this);
            else if (parent)
                parent->remove(this);
        });
    });
    m_close_button->add_class("window-button");
    m_close_button->add_class("window-close-button");

    initialize_context();
    set_floating(floating);
}

void bgui::window::initialize_context() {
    m_context = &add_persistent<bgui::linear>(bgui::orientation::vertical);
    m_context->style.layout.require_mode(bgui::mode::match_parent, bgui::mode::match_parent);
    m_context->style.visual.visible = false;
}
void bgui::window::on_update() {
    if (is_floating()) {
        const auto drag = m_title->is_drag();
        if(drag[0] || drag[1]) {
            set_position(processed_x() + drag[0], processed_y() + drag[1]);
            m_dragging = true;
        } else if (!bgui::get_pressed(bgui::input_key::mouse_left)) {
            m_dragging = false;
        }
    } else {
        const auto drag = m_title->is_drag();
        if (bgui::get_pressed(bgui::input_key::mouse_left) && (drag.x != 0 || drag.y != 0)) {
            m_pinned_drag_distance.x += drag.x;
            m_pinned_drag_distance.y += drag.y;
            const int threshold = std::max(12, static_cast<int>(24.f * bgui::get_global_scale()));
            if (m_pinned_drag_distance.x * m_pinned_drag_distance.x +
                m_pinned_drag_distance.y * m_pinned_drag_distance.y >= threshold * threshold) {
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

    const int header_height = m_header->processed_height();
    const float scale = bgui::get_global_scale();
    if (header_height > 0 && scale > 0.f && header_height != m_icon_height) {
        const int icon_width = std::max(1, static_cast<int>(header_height * m_icon_aspect_ratio));
        m_icon->set_size(icon_width / scale, header_height / scale);
        m_icon_height = header_height;
    }
}

void bgui::window::set_icon(const std::string& path) {
    const auto& texture = bgui::asset_manager::get_instance().load_texture(path);
    m_icon->set_texture(texture);
    if (texture.m_size.y > 0.f)
        m_icon_aspect_ratio = texture.m_size.x / texture.m_size.y;
    m_icon_height = 0;
}
void bgui::window::set_floating(bool floating) {
    if (floating && !m_floating) {
        const float scale = bgui::get_global_scale();
        if (scale > 0.f && processed_width() > 0 && processed_height() > 0) {
            style.layout.require_mode(bgui::mode::pixel, bgui::mode::pixel);
            style.layout.require_size(processed_width() / scale, processed_height() / scale);
            mark_style_dirty();
        }
    }
    m_floating = floating;
    set_flex(!floating);
    if (m_close_button)
        m_close_button->set_enable(floating);
    set_resizable(floating);
}

void bgui::window::set_tabbed(bool tabbed) {
    if (m_header)
        m_header->set_enable(!tabbed);
}
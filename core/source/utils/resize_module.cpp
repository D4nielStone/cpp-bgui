#include "utils/resize_module.hpp"
#include "bgui.hpp"
#include "lay/layout.hpp"
#include "os/os.hpp"
#include <algorithm>

namespace bgui {
    namespace {
        class resize_handle : public element {
        private:
            layout* m_owner;
            int m_horizontal;
            int m_vertical;

            void update_cursor() const {
                auto& context = bgui::get_context();
                if (m_horizontal == 0) {
                    context.m_actual_cursor = cursor::resize_vertical;
                } else if (m_vertical == 0) {
                    context.m_actual_cursor = cursor::resize_horizontal;
                } else if (m_horizontal == m_vertical) {
                    context.m_actual_cursor = cursor::resize_nwse;
                } else {
                    context.m_actual_cursor = cursor::resize_nesw;
                }
            }

        public:
            resize_handle(layout* owner, int horizontal, int vertical)
                : m_owner(owner), m_horizontal(horizontal), m_vertical(vertical) {
                type = "resize_handle";
                add_class("resize-handle");
                if (horizontal < 0) add_class("resize-left");
                if (horizontal > 0) add_class("resize-right");
                if (vertical < 0) add_class("resize-top");
                if (vertical > 0) add_class("resize-bottom");
                set_flex(false);
                style.layout.require_size(8.f, 8.f);
                style.layout.require_mode(mode::pixel, mode::pixel);
            }

            void on_update() override {
                const int thickness = std::max(1, static_cast<int>(8.f * get_global_scale()));
                const int owner_x = m_owner->processed_x();
                const int owner_y = m_owner->processed_y();
                const int owner_width = m_owner->processed_width();
                const int owner_height = m_owner->processed_height();
                int x = owner_x;
                int y = owner_y;
                int width = thickness;
                int height = thickness;

                if (m_horizontal == 0) {
                    width = std::max(0, owner_width - 2 * thickness);
                    x += thickness;
                } else if (m_horizontal > 0) {
                    x += owner_width - thickness;
                }

                if (m_vertical == 0) {
                    height = std::max(0, owner_height - 2 * thickness);
                    y += thickness;
                } else if (m_vertical > 0) {
                    y += owner_height - thickness;
                }

                set_final_rect(x, y, width, height);

                const vec2i delta = is_drag();
                if (delta.x != 0 || delta.y != 0) {
                    const auto limits = m_owner->computed_style.layout;
                    const float scale = get_global_scale();
                    auto& required_size = m_owner->style.layout;
                    if (!required_size.size_mode)
                        required_size.size_mode = limits.size_mode;
                    if (!required_size.size) {
                        vec2 initial_size = limits.size;
                        for (size_t axis = 0; axis < 2; ++axis) {
                            if (limits.size_mode[axis] == mode::pixel)
                                initial_size[axis] /= scale;
                        }
                        required_size.size = initial_size;
                    }

                    if (m_horizontal != 0) {
                        const int new_width = std::clamp(
                            owner_width + m_horizontal * delta.x,
                            limits.limit_min.x, limits.limit_max.x);
                        (*required_size.size_mode)[0] = mode::pixel;
                        (*required_size.size)[0] = new_width / scale;
                        if (m_horizontal < 0 && !m_owner->is_flex())
                            m_owner->set_position(owner_x + owner_width - new_width, owner_y);
                    }
                    if (m_vertical != 0) {
                        const int new_height = std::clamp(
                            owner_height + m_vertical * delta.y,
                            limits.limit_min.y, limits.limit_max.y);
                        (*required_size.size_mode)[1] = mode::pixel;
                        (*required_size.size)[1] = new_height / scale;
                        if (m_vertical < 0 && !m_owner->is_flex())
                            m_owner->set_position(m_owner->processed_x(), owner_y + owner_height - new_height);
                    }
                    m_owner->mark_style_dirty();
                    set_drag({0, 0});
                }
            }

            void on_mouse_hover() override {
                element::on_mouse_hover();
                update_cursor();
            }

            void on_pressed() override {
                element::on_pressed();
                update_cursor();
            }
        };
    }

    void resize_module::configure(layout& owner, bool enabled) {
        if (!enabled) {
            for (auto& [lay, elements] : owner.get_elements()) {
                elements.erase(std::remove_if(elements.begin(), elements.end(),
                    [](const std::unique_ptr<element>& elem) {
                        return elem->type == "resize_handle";
                    }), elements.end());
            }
            return;
        }

        bool found_handles = false;
        for (auto& [lay, elements] : owner.get_elements()) {
            for (auto& elem : elements) {
                if (elem->type != "resize_handle") continue;
                found_handles = true;
            }
        }

        if (found_handles) return;

        for (int vertical : {-1, 0, 1}) {
            for (int horizontal : {-1, 0, 1}) {
                if (horizontal == 0 && vertical == 0) continue;
                owner.add<resize_handle, layer::overlay>(&owner, horizontal, vertical);
            }
        }
    }
}
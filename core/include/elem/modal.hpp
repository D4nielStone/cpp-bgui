#pragma once

#include "elem/button.hpp"

#include <functional>
#include <string>
#include <utility>

namespace bgui {
    class modal : public linear {
    public:
        modal();

        template<typename T, layer Lay = layer::base, typename... Args>
        scoped_element<T> add(Args&&... args) {
            auto item = m_content->add<T, Lay>(std::forward<Args>(args)...);
            configure_content_item(*item);
            return item;
        }

        template<typename T, layer Lay = layer::base, typename... Args>
        T& add_persistent(Args&&... args) {
            T& item = m_content->add_persistent<T, Lay>(
                std::forward<Args>(args)...
            );
            configure_content_item(item);
            return item;
        }

        linear& content() { return *m_content; }
        void set_on_confirm(std::function<void()> callback);
        void set_confirmation_text(const std::string& text);

    private:
        linear* m_panel{nullptr};
        linear* m_content{nullptr};
        linear* m_actions{nullptr};
        button* m_confirm_button{nullptr};
        std::function<void()> m_on_confirm;

        template<typename T>
        static void configure_content_item(T& item) {
            if (item.type == "text")
                item.add_class("modal-text");

            if (!item.style.layout.size_mode) {
                item.style.layout.require_mode(
                    mode::wrap_content,
                    mode::wrap_content
                );
            }
        }

        void confirm();
    };
}

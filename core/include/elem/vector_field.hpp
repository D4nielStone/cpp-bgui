#pragma once

#include "elem/inputbox.hpp"

namespace bgui {
    class vector_field : public linear {
    public:
        explicit vector_field(
            const std::string& label,
            const std::vector<std::string>& axes,
            float scale = 0.4f
        );

        std::size_t get_component_count() const noexcept {
            return m_components.size();
        }

        inputbox& get_component(std::size_t index) {
            return *m_components.at(index);
        }

        const inputbox& get_component(std::size_t index) const {
            return *m_components.at(index);
        }

    private:
        std::vector<inputbox*> m_components;
    };
}

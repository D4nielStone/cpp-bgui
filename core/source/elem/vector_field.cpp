#include "elem/vector_field.hpp"

namespace bgui {
    vector_field::vector_field(
        const std::string& label,
        const std::vector<std::string>& axes,
        const float scale
    ) : linear(orientation::vertical) {
        type = "vector_field";
        style.layout.require_mode(mode::match_parent, mode::wrap_content);

        auto& field_label = add_persistent<text>(label, scale);
        field_label.style.layout.require_mode(mode::match_parent, mode::wrap_content);
        field_label.style.layout.align = vec<2UL, alignment>(
            {alignment::center, alignment::start}
        );

        auto& values = add_persistent<linear>(orientation::horizontal);
        values.style.layout.require_mode(mode::match_parent, mode::wrap_content);

        m_components.reserve(axes.size());
        for (const auto& axis : axes) {
            values.add_persistent<text>(axis, scale);
            auto& input = values.add_persistent<inputbox>(
                "", "", scale, nullptr, input_mode::number
            );
            input.style.layout.require_width(mode::stretch);
            input.style.layout.require_height(mode::pixel, 20.f);
            m_components.push_back(&input);
        }
    }
}

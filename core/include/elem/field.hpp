#pragma once

#include "inputbox.hpp"

namespace bgui {

class field : public linear {
protected:
    text* m_label;
    inputbox* m_inputbox;

public:
    explicit field(
        const std::string& label,
        const std::string& buffer,
        float scale,
        const std::function<void(const std::string)>& action = nullptr,
        const std::string& placeholder = "",
        input_mode mode = input_mode::inputbox
    );

    text& get_label()
    {
        return *m_label;
    }

    inputbox& get_inputbox()
    {
        return *m_inputbox;
    }
};

} // namespace bgui
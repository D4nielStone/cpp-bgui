#include <bgui.hpp>

namespace bgui {

field::field(
    const std::string& label,
    const std::string& buffer,
    const float scale,
    const std::function<void(const std::string)>& action,
    const std::string& placeholder,
    const input_mode mode
)
    : linear(orientation::horizontal),
      m_label(nullptr),
      m_inputbox(nullptr)
{
    style.layout.require_mode(mode::stretch, mode::wrap_content);
    m_label = &add_persistent<bgui::text>(label, scale);
    m_label->add_class("inputbox-label");
    m_inputbox = &add_persistent<bgui::inputbox>(buffer, placeholder, scale, action, mode);
}
}
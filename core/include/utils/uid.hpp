#pragma once
#include <string>

namespace bgui {
    class uid {
    private:
        inline static unsigned int s_nextid {0};
        unsigned int m_uint;
        std::string m_str;
    public:
        // @brief  Default Constructor
        // Just set the id and increment to the next.
        uid() : m_uint(s_nextid++), m_str("") {}
        // @brief Tag Constructor
        // Set one tag
        uid(const std::string& s)  : m_uint(s_nextid++), m_str(s) {}

        std::string get() const {
            return m_str + "#" + std::to_string(m_uint);
        };
    };
}
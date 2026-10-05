#pragma once

#include <ostream>

namespace bgui {
    void set_logs_enabled(bool enabled) noexcept;
    bool logs_enabled() noexcept;

    namespace detail {
        std::ostream& log_out() noexcept;
        std::ostream& log_err() noexcept;
    }
}
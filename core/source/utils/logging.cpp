#include "utils/logging.hpp"

#include <atomic>
#include <iostream>
#include <ostream>
#include <streambuf>

namespace {
    class null_streambuf final : public std::streambuf {
    protected:
        int_type overflow(const int_type character) override {
            return traits_type::not_eof(character);
        }

        int sync() override {
            return 0;
        }
    };

    std::ostream& null_stream() {
        static thread_local null_streambuf buffer;
        static thread_local std::ostream stream(&buffer);
        return stream;
    }

#ifdef BGUI_LOGS_DEFAULT_ENABLED
    std::atomic_bool s_logs_enabled{true};
#else
    std::atomic_bool s_logs_enabled{false};
#endif
}

void bgui::set_logs_enabled(const bool enabled) noexcept {
    s_logs_enabled.store(enabled, std::memory_order_relaxed);
}

bool bgui::logs_enabled() noexcept {
    return s_logs_enabled.load(std::memory_order_relaxed);
}

std::ostream& bgui::detail::log_out() noexcept {
    return logs_enabled() ? std::cout : null_stream();
}

std::ostream& bgui::detail::log_err() noexcept {
    return logs_enabled() ? std::cerr : null_stream();
}
#pragma once
#include "element.hpp"

namespace elements {
    class box : public element {
    public:
        box();
        ~box() = default;

        void update() override;
        void set_theme(const butil::theme& t) override;
    };
} // namespace elements
#pragma once

namespace bgui {
    class layout;

    class resize_module {
    public:
        static void configure(layout& owner, bool enabled);
    };
}
#pragma once

namespace bgui {
    enum class cursor {
        arrow,
        hand,
        ibeam
    };
    enum class input_key {
        none,
        mouse_left,
        mouse_right,
        mouse_middle
    };
    enum class input_action {
        none,
        press,
        release,
        repeat
    };
    enum class state {
        normal,
        hover,
        pressed,
        focused,
        disabled
    };
    enum class mode {
        percent,
        pixel,
        match_parent,
        wrap_content,
        stretch,
        same
    };
    enum class orientation {
        vertical,
        horizontal
    };
    enum class alignment {
        start,
        center,
        end
    };
}
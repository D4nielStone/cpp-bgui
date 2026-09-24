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
        mouse_middle,
        backspace,
        enter,
        left,
        right,
        escape
        ,q, w, e, r, t, y, u, i, o, p
        ,a, s, d, f, g, h, j, k, l
        ,z, x, c, v, b, n, m
        ,up, down, left_shift, right_shift
        ,left_control, right_control, left_alt, right_alt
        ,delete_key, f5, keypad_enter
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
#version 330 core

in vec2 Uv; // UV coordinates from vertex shader (0..1)
out vec4 FragColor;

// Uniforms
uniform vec4 u_bg_color;
uniform vec4 u_border_color;
uniform bool u_bordered;
uniform float u_border_radius;
uniform int u_border_size;
uniform vec4 u_rect;

void main() {
    if(u_bordered) {
        vec2 pos = u_rect.zw * Uv;
        vec2 halfSize = u_rect.zw / 2;
        vec2 d = abs(pos - halfSize)  - (halfSize - vec2(u_border_radius));
        float dist = length(max(d, 0.0)) - u_border_radius;
        if(dist > 0.0) discard;
        if (dist > -u_border_size && u_bordered) {
            FragColor = u_border_color;
        } else {
            FragColor = u_bg_color;
        }
    } else {
        FragColor = u_bg_color;
    }
}
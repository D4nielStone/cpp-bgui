#version 330 core

in vec2 Uv;
out vec4 FragColor;

uniform sampler2D u_texture;
vec4 u_text_color = vec4(1 , 1, 1, 1);     // RGBA text color
uniform vec2 u_uv_min;
uniform vec2 u_uv_max;

void main() {
    vec2 uv = mix(u_uv_min, u_uv_max, vec2(Uv.x, 1-Uv.y));

    float dist = texture(u_texture, uv).r;
    FragColor = vec4(u_text_color.rgb, u_text_color.a * dist);
}
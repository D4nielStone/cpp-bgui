#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;

out vec2 Uv;

uniform vec4 u_rect;
uniform mat4 u_projection;

void main() {
    vec2 pos = aPos * u_rect.zw + u_rect.xy;
    Uv = aUv;
    gl_Position =  u_projection * vec4(pos, 0, 1);
}
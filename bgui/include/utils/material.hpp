#pragma once
#include "gl/shader.hpp"
#include <unordered_map>

namespace butil {
    class material {
    public:
        bgl::shader m_shader;
        std::unordered_map<std::string, bgl::uniform> m_uniforms;
        GLuint m_texture;
        bool m_visible;

        material();
        ~material();
        void bind_uniforms();
        void set(const std::string& name, const bgl::uniform& u);

        bool operator==(const material &other) const {
            return m_uniforms == other.m_uniforms;
        };
    };
}
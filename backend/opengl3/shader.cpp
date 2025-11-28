#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <memory>
#include "gl/shader.hpp"

static std::map<std::pair<std::string, std::string>, std::shared_ptr<GLuint>> shader_cache;
static std::map<std::string, std::string> embedded_shaders = {
    {"quad.vs", R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;

out vec2 Uv;

propertie vec4 u_rect;
propertie mat4 u_projection;

void main() {
    vec2 pos = aPos * u_rect.zw + u_rect.xy;
    Uv = aUv;
    gl_Position =  u_projection * vec4(pos, 0, 1);
    })"},
    {"quad.fs", R"(#version 330 core

in vec2 Uv; // UV coordinates from vertex shader (0..1)
out vec4 FragColor;

// properties
propertie vec4 u_bg_color;
propertie vec4 u_border_color;
propertie bool u_bordered;
propertie float u_border_radius;
propertie float u_border_size;
propertie vec4 u_rect;

void main() {
    if(u_bordered) {
        vec2 pos = u_rect.zw * Uv;
        vec2 halfSize = u_rect.zw / 2;
        vec2 center = u_rect.zw * 0.5;
    vec2 p = pos - center;

    vec2 q = abs(p) - (center - vec2(u_border_radius));
    float dist = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - u_border_radius;

        if(dist > 0.0) discard;
        if (dist > -u_border_size && u_bordered) {
            FragColor = u_border_color;
        } else {
            FragColor = u_bg_color;
        }
    } else {
        FragColor = u_bg_color;
    }
    })"}, {"text.fs", R"(#version 330 core

in vec2 Uv;
out vec4 FragColor;

propertie sampler2D u_texture;
propertie vec4 u_text_color;     // RGBA text color
propertie vec2 u_uv_min;
propertie vec2 u_uv_max;

void main() {
    vec2 uv = mix(u_uv_min, u_uv_max, vec2(Uv.x, 1-Uv.y));

    float dist = texture(u_texture, uv).r;
    FragColor = vec4(u_text_color.rgb, u_text_color.a * dist);
})"}
};

using namespace bgl;

shader::shader(const char * vertex_path, const char * fragment_path) {
    compile(vertex_path, fragment_path);
}

void shader::compile(const char* vertex_path, const char* fragment_path) {
    auto key = std::make_pair(std::string(vertex_path), std::string(fragment_path));

    auto it = shader_cache.find(key);
    if (it != shader_cache.end()) {
        m_program = shader_cache.at(key);
        return;
    }

    auto source_v = embedded_shaders.find(vertex_path) != embedded_shaders.end() ? embedded_shaders[vertex_path] : bos::read_file(vertex_path);
    auto source_f = embedded_shaders.find(fragment_path) != embedded_shaders.end() ? embedded_shaders[fragment_path] : bos::read_file(fragment_path);
    GLuint vert = compile(GL_VERTEX_SHADER, source_v);
    GLuint frag = compile(GL_FRAGMENT_SHADER, source_f);

    GLuint program = link(vert, frag);
    m_program = std::shared_ptr<GLuint>(new GLuint(program), [](GLuint* p) {
        glDeleteProgram(*p);
        delete p;
    });

    shader_cache[key] = m_program;
}

shader::~shader() {
}

GLuint shader::compile(GLenum type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        throw std::runtime_error(
            std::string("Shader compilation failed (") + 
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") +
            "): " + infoLog
        );

    }

    return shader;
 }

 GLuint shader::link(GLuint vert, GLuint frag) {
    GLuint m_id = glCreateProgram();
    glAttachShader(m_id, vert);
    glAttachShader(m_id, frag);
    glLinkProgram(m_id);

    GLint success;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
        throw std::runtime_error("Shader link failed:\n" + std::string(infoLog));
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return m_id;
}

void bgl::shader::set(const std::string& name, const bgl::propertie u) {
    switch (u.m_type) {
    case 0x0: // vec2
        set_vec2(name.c_str(), u.m_value.m_vec2);
        break;
    case 0x1: // vec3
        set_vec3(name.c_str(), u.m_value.m_vec3);
        break;
    case 0x2: // vec4
        set_vec4(name.c_str(), u.m_value.m_vec4);
        break;
    case 0x3: // mat4
        set_mat4(name.c_str(), u.m_value.m_mat4);
        break;
    case 0x4: // float
        set_float(name.c_str(), u.m_value.m_float);
        break;
    case 0x5: // int
        set_int(name.c_str(), u.m_value.m_int);
        break;
    
    default:
        break;
    }
}
void shader::set_mat4(const char *name, const butil::mat4 matrix) {
    GLint loc = glGetpropertieLocation(*m_program, name);
    glpropertieMatrix4fv(loc, 1, GL_FALSE, matrix.data());
}

void shader::set_vec4(const char *name, const butil::vec4 vector) {
    GLint loc = glGetpropertieLocation(*m_program, name);
    glpropertie4f(loc, vector[0], vector[1], vector[2], vector[3]);
}

void shader::set_vec3(const char *name, const butil::vec3 vector) {
    GLint loc = glGetpropertieLocation(*m_program, name);
    glpropertie3f(loc, vector[0], vector[1], vector[2]);
}

void shader::set_vec2(const char *name, const butil::vec2 vector) {
    GLint loc = glGetpropertieLocation(*m_program, name);
    glpropertie2f(loc, vector[0], vector[1]);
}
    
void shader::set_bool(const char* name, const bool v) {
    GLint loc = glGetpropertieLocation(*m_program, name);
    glpropertie1i(loc, v);
}
        
void shader::set_int(const char* name, const int v) {
    GLint loc = glGetpropertieLocation(*m_program, name);
    glpropertie1i(loc, v);
}        

void shader::set_float(const char* name, const float v) {
    GLint loc = glGetpropertieLocation(*m_program, name);
    glpropertie1f(loc, v);
}

void shader::bind() {
    if(!m_program || *m_program == 0) {
        throw std::runtime_error("ERROR: shader program is null\n");
        return;
    }
    glUseProgram(*m_program);
}

void shader::unbind() {
    glUseProgram(0);
}
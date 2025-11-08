#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include "gl/shader.hpp"

static std::map<std::pair<std::string, std::string>, GLuint> shader_cache;

shader::shader(const char * vertex_path, const char * fragment_path) {
    compile(vertex_path, fragment_path);
}

void shader::compile(const char * vertex_path, const char * fragment_path) {
    bool vert_found = (shader_cache.find({vertex_path, fragment_path}) != shader_cache.end());
    
    if (vert_found) {
        m_id = shader_cache[{vertex_path, fragment_path}];
        return;
    }

    GLuint vert = compile(GL_VERTEX_SHADER, bgui_os::read_file(vertex_path));
    GLuint frag =  compile(GL_FRAGMENT_SHADER, bgui_os::read_file(fragment_path));

    m_id = link(vert, frag);
    shader_cache[{vertex_path, fragment_path}] = m_id;
}

shader::~shader() {
    glDeleteProgram(m_id);
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
        throw std::runtime_error("Shader compilation failed:\n" + std::string(infoLog));
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

void shader::set_mat4(const char *name, const bgui_utils::mat4 &matrix) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.data());
}

void shader::set_vec4(const char *name, const bgui_utils::vec4 &vector) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform4f(loc, vector[0], vector[1], vector[2], vector[3]);
}

void shader::set_vec3(const char *name, const bgui_utils::vec3 &vector) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform3f(loc, vector[0], vector[1], vector[2]);
}

void shader::set_vec2(const char *name, const bgui_utils::vec2 &vector) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform2f(loc, vector[0], vector[1]);
}

void shader::bind() {
    glUseProgram(m_id);
}

void shader::unbind() {
    glUseProgram(0);
}
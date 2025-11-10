#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include <iostream>
#include "gl/shader.hpp"

static std::map<std::pair<std::string, std::string>, GLuint> shader_cache;
using namespace bgl;

shader::shader(const char * vertex_path, const char * fragment_path) {
    compile(vertex_path, fragment_path);
}

void shader::compile(const char * vertex_path, const char * fragment_path) {
    bool vert_found = (shader_cache.find({vertex_path, fragment_path}) != shader_cache.end());
    
    if (vert_found) {
        m_id = shader_cache[{vertex_path, fragment_path}];
        m_should_delete = false;
        return;
    }

    GLuint vert = compile(GL_VERTEX_SHADER, bos::read_file(vertex_path));
    GLuint frag =  compile(GL_FRAGMENT_SHADER, bos::read_file(fragment_path));

    m_id = link(vert, frag);
    m_should_delete = true;

    shader_cache[{vertex_path, fragment_path}] = m_id;
}

shader::~shader() {
    if(m_should_delete)
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

void bgl::shader::set(const std::string& name, const bgl::uniform &u) {
    std::cout << "new set: " << name << " type: " << std::to_string(u.m_type) << "\n";
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
    GLint loc = glGetUniformLocation(m_id, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.data());
}

void shader::set_vec4(const char *name, const butil::vec4 &vector) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform4f(loc, vector[0], vector[1], vector[2], vector[3]);
}

void shader::set_vec3(const char *name, const butil::vec3 &vector) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform3f(loc, vector[0], vector[1], vector[2]);
}

void shader::set_vec2(const char *name, const butil::vec2 &vector) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform2f(loc, vector[0], vector[1]);
}
    
void shader::set_bool(const char* name, const bool& v) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform1i(loc, v);
}
        
void shader::set_int(const char* name, const int& v) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform1i(loc, v);
}        

void shader::set_float(const char* name, const float& v) {
    GLint loc = glGetUniformLocation(m_id, name);
    glUniform1f(loc, v);
}

void shader::bind() {
    glUseProgram(m_id);
}

void shader::unbind() {
    glUseProgram(0);
}
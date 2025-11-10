#pragma once
#include <glad/glad.h>
#include <os/os.hpp>
#include "utils/mat.hpp"
#include "utils/vec.hpp"

namespace bgl {
    union any_t {
        butil::vec2 m_vec2;
        butil::vec3 m_vec3;
        butil::vec4 m_vec4;
        butil::mat4 m_mat4;
        float m_float;
        int m_int;
        any_t() {};
        ~any_t(){};
    };

    struct uniform {
        uint8_t m_type; /// < the type of the uniform
        any_t m_value;
        // constructors:
        uniform() {m_value.m_int = 0;};
        uniform(const butil::vec2& v) : m_type(0x0){m_value.m_vec2 = v;};
        uniform(const butil::vec3& v) : m_type(0x1){m_value.m_vec3 = v;};
        uniform(const butil::vec4& v) : m_type(0x2){m_value.m_vec4 = v;};
        uniform(const butil::mat4& v) : m_type(0x3){m_value.m_mat4 = v;};
        uniform(const float& v) :  m_type(0x4){m_value.m_float = v;};
        uniform(const int& v) : m_type(0x5){m_value.m_int = v;};
        uniform& operator=(const uniform& other) {
            switch (m_type) {
            case 0x0:
                m_value.m_vec2 = other.m_value.m_vec2;
                break;
            case 0x1:
                m_value.m_vec3 = other.m_value.m_vec3;
                break;
            case 0x2:
                m_value.m_vec4 = other.m_value.m_vec4;
                break;
            case 0x3:
                m_value.m_mat4 = other.m_value.m_mat4;
                break;
            case 0x4:
                m_value.m_float = other.m_value.m_float;
                break;
            case 0x5:
                m_value.m_int = other.m_value.m_int;
                break;
            
            default:
                break;
            }
            m_type = other.m_type;
            return *this;
        }
        bool operator==(const uniform& other) const {
            bool v = true;
            switch (m_type) {
            case 0x0:
                v = m_value.m_vec2 == other.m_value.m_vec2;
                break;
            case 0x1:
                v = m_value.m_vec3 == other.m_value.m_vec3;
                break;
            case 0x2:
                v = m_value.m_vec4 == other.m_value.m_vec4;
                break;
            case 0x3:
                v = m_value.m_mat4 == other.m_value.m_mat4;
                break;
            case 0x4:
                v = m_value.m_float == other.m_value.m_float;
                break;
            case 0x5:
                v = m_value.m_int == other.m_value.m_int;
                break;
            
            default:
                break;
            }
            return (m_type == other.m_type && v);
        }
    };

    class shader {
    private:
        GLuint m_id{0};
        bool m_should_delete{false};
        
        void set_mat4(const char* name, const butil::mat4 matrix);
        void set_vec4(const char* name, const butil::vec4& vector);
        void set_vec3(const char* name, const butil::vec3& vector);
        void set_vec2(const char* name, const butil::vec2& vector);
        void set_bool(const char* name, const bool& v);
        void set_int(const char* name, const int& v);
        void set_float(const char* name, const float& v);
    public:
        shader() = default;
        shader(const char* vertex_path, const char* fragment_path);
        ~shader();
        void bind();
        void unbind();

        void compile(const char* vertex_path, const char* fragment_path);
        static GLuint compile(GLenum type, const std::string& source);
        static GLuint link(GLuint vert, GLuint frag);

        void set(const std::string& name, const uniform& u);

        bool operator==(const shader& other) const {
            return m_id == other.m_id;
        }
    };
} // namespace bgl
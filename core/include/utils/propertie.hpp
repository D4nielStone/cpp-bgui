#pragma once
#include "vec.hpp"
#include "mat.hpp"

namespace butil {
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

    struct propertie {
        char m_type; /// < the type of the propertie
        any_t m_value;
        // constructors:
        propertie() : m_type(0x5) {m_value.m_int = 0;};
        propertie(const butil::vec2& v) : m_type(0x0){m_value.m_vec2 = v;};
        propertie(const butil::vec3& v) : m_type(0x1){m_value.m_vec3 = v;};
        propertie(const butil::vec4& v) : m_type(0x2){m_value.m_vec4 = v;};
        propertie(const butil::mat4& v) : m_type(0x3){m_value.m_mat4 = v;};
        propertie(const float& v) :  m_type(0x4){m_value.m_float = v;};
        propertie(const int& v) : m_type(0x5){m_value.m_int = v;};
        propertie& operator=(const propertie& other) {
            if (this == &other) return *this;
            m_type = other.m_type;
            switch (m_type) {
                case 0x0: m_value.m_vec2 = other.m_value.m_vec2; break;
                case 0x1: m_value.m_vec3 = other.m_value.m_vec3; break;
                case 0x2: m_value.m_vec4 = other.m_value.m_vec4; break;
                case 0x3: m_value.m_mat4 = other.m_value.m_mat4; break;
                case 0x4: m_value.m_float = other.m_value.m_float; break;
                case 0x5: m_value.m_int = other.m_value.m_int; break;
                default: break;
            }
            return *this;
        }

        bool operator==(const propertie& other) const {
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
};
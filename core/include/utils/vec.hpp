#pragma once
#include <cmath>
#include <array>

namespace bgui {
    template<size_t N, typename T = float>
    class vec {
    public:
        std::array<T, N> v{};

        vec() {
            for(auto& component : v) {
                component = (T)0;
            } 
        }
        
        vec(const T& scalar) {
            for(auto& component : v) {
                component = scalar;
            } 
        }

        vec(std::initializer_list<T> list) {
            size_t i = 0;
            for (auto value : list) {
                if (i < N)
                    v[i++] = value;
            }
            for (; i < N; ++i)
                v[i] = T(0);
        }

        T& operator[](size_t index) {
            return v[index];
        }
        const T& operator[](size_t index) const {
            return v[index];
        }

        vec<N, T> operator+(const vec<N, T>& other) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = v[i] + other[i];
            }
            return result;
        }

        vec<N, T> operator-(const vec<N, T>& other) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = v[i] - other[i];
            }
            return result;
        }

        vec<N, T> operator*(const T& scalar) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = v[i] * scalar;
            }
            return result;
        }

        vec<N, T> operator/(const T& scalar) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = v[i] / scalar;
            }
            return result;
        }

        vec<N, T> operator*(const vec<N, T>& other) const {
            vec result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = v[i] * other[i];
            }
            return result;
        }

        bool operator==(const vec<N, T>& other) const {
            return v == other.v;
        }
    };
    template<typename T>
    class vec<4, T> {
    public:
        union {
            struct { T x, y, z, w; };
            struct { T r, g, b, a; };
            std::array<T, 4> v;
        };

        vec() : x(0), y(0), z(0), w(0) {}

        vec(T scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

        vec(std::initializer_list<T> list) {
            auto it = list.begin();
            x = (it != list.end()) ? *it++ : T(0);
            y = (it != list.end()) ? *it++ : T(0);
            z = (it != list.end()) ? *it++ : T(0);
            w = (it != list.end()) ? *it++ : T(0);
        }

        T& operator[](size_t i) { return v[i]; }
        const T& operator[](size_t i) const { return v[i]; }

        bool operator==(const vec& other) const {
            return v == other.v;
        }
        vec<4, T> operator+(const vec<4, T>& other) const {
            vec<4, T> result;
            for (size_t i = 0; i < 4; ++i) {
                result[i] = v[i] + other[i];
            }
            return result;
        }

        vec<4, T> operator-(const vec<4, T>& other) const {
            vec<4, T> result;
            for (size_t i = 0; i < 4; ++i) {
                result[i] = v[i] - other[i];
            }
            return result;
        }

        vec<4, T> operator*(const T& scalar) const {
            vec<4, T> result;
            for (size_t i = 0; i < 4; ++i) {
                result[i] = v[i] * scalar;
            }
            return result;
        }
        vec<4, T> operator/(const T& scalar) const {
            vec<4, T> result;
            for (size_t i = 0; i < 4; ++i) {
                result[i] = v[i] / scalar;
            }
            return result;
        }
        vec<4, T> operator*(const vec<4, T>& other) const {
            vec<4, T> result;
            for (size_t i = 0; i < 4; ++i) {
                result[i] = v[i] * other[i];
            }
            return result;
        }
    };

    template<typename T>
    class vec<3, T> {
    public:
        union {
            struct { T x, y, z; };
            struct { T r, g, b; };
            std::array<T, 3> v;
        };

        vec() : x(0), y(0), z(0) {}

        vec(T scalar) : x(scalar), y(scalar), z(scalar) {}

        vec(std::initializer_list<T> list) {
            auto it = list.begin();
            x = (it != list.end()) ? *it++ : T(0);
            y = (it != list.end()) ? *it++ : T(0);
            z = (it != list.end()) ? *it++ : T(0);
        }

        T& operator[](size_t i) { return v[i]; }
        const T& operator[](size_t i) const { return v[i]; }

        bool operator==(const vec& other) const {
            return v == other.v;
        }
        vec<3, T> operator+(const vec<3, T>& other) const {
            vec<3, T> result;
            for (size_t i = 0; i < 3; ++i) {
                result[i] = v[i] + other[i];
            }
            return result;
        }

        vec<3, T> operator-(const vec<3, T>& other) const {
            vec<3, T> result;
            for (size_t i = 0; i < 3; ++i) {
                result[i] = v[i] - other[i];
            }
            return result;
        }

        vec<3, T> operator*(const T& scalar) const {
            vec<3, T> result;
            for (size_t i = 0; i < 3; ++i) {
                result[i] = v[i] * scalar;
            }
            return result;
        }
        vec<3, T> operator/(const T& scalar) const {
            vec<3, T> result;
            for (size_t i = 0; i < 3; ++i) {
                result[i] = v[i] / scalar;
            }
            return result;
        }
        vec<3, T> operator*(const vec<3, T>& other) const {
            vec<3, T> result;
            for (size_t i = 0; i < 3; ++i) {
                result[i] = v[i] * other[i];
            }
            return result;
        }

        vec<3, T> cross(const vec<3, T>& other) const {
            return vec<3, T>{
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            };
        }

        vec<3, T> normalized() const {
            T len = std::sqrt(x * x + y * y + z * z);
            if (len > 0) {
                return vec<3, T>{x / len, y / len, z / len};
            }
            return vec<3, T>{0, 0, 0};
        }

        vec<3, T> operator+=(const vec<3, T>& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        vec<3, T> operator-=(const vec<3, T>& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        vec<3, T> operator/=(const T& scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        vec<3, T> operator*=(const T& scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        float length() const {
            return std::sqrt(x * x + y * y + z * z);
        }
    };

    template<typename T>
    class vec<2, T> {
    public:
        union {
            struct { T x, y, z; };
            struct { T r, g, b; };
            std::array<T, 2> v;
        };

        vec() : x(0), y(0), z(0) {}

        vec(T scalar) : x(scalar), y(scalar), z(scalar) {}

        vec(std::initializer_list<T> list) {
            auto it = list.begin();
            x = (it != list.end()) ? *it++ : T(0);
            y = (it != list.end()) ? *it++ : T(0);
            z = (it != list.end()) ? *it++ : T(0);
        }

        T& operator[](size_t i) { return v[i]; }
        const T& operator[](size_t i) const { return v[i]; }

        bool operator==(const vec& other) const {
            return v == other.v;
        }
        vec<2, T> operator+(const vec<2, T>& other) const {
            vec<2, T> result;
            for (size_t i = 0; i < 2; ++i) {
                result[i] = v[i] + other[i];
            }
            return result;
        }

        vec<2, T> operator-(const vec<2, T>& other) const {
            vec<2, T> result;
            for (size_t i = 0; i < 2; ++i) {
                result[i] = v[i] - other[i];
            }
            return result;
        }

        vec<2, T> operator*(const T& scalar) const {
            vec<2, T> result;
            for (size_t i = 0; i < 2; ++i) {
                result[i] = v[i] * scalar;
            }
            return result;
        }
        vec<2, T> operator/(const T& scalar) const {
            vec<2, T> result;
            for (size_t i = 0; i < 2; ++i) {
                result[i] = v[i] / scalar;
            }
            return result;
        }
        vec<2, T> operator*(const vec<2, T>& other) const {
            vec<2, T> result;
            for (size_t i = 0; i < 2; ++i) {
                result[i] = v[i] * other[i];
            }
            return result;
        }
    };
    
    using vec4 = vec<4, float>;
    using vec3 = vec<3, float>;
    using vec2 = vec<2, float>;

    using vec4i = vec<4, int>;
    using vec3i = vec<3, int>;
    using vec2i = vec<2, int>;

    using color = vec4;
} // namespace bgui
#pragma once
#include <array>
#include <cstddef>

namespace butil {

template<size_t R, size_t C, typename T>
class mat {
public:
    std::array<T, R * C> m{};

    mat() {
        for (auto& v : m) v = T(0);
        if (R == C) {
            for (size_t i = 0; i < R; ++i)
                (*this)(i, i) = T(1);
        }
    }

    T& operator()(size_t row, size_t col) { return m[row * C + col]; }
    const T& operator()(size_t row, size_t col) const { return m[row * C + col]; }
    const T* data() const { return m.data(); }

    template<size_t C2>
    mat<R, C2, T> operator*(const mat<C, C2, T>& other) const {
        mat<R, C2, T> result;
        for (size_t i = 0; i < R; ++i)
            for (size_t j = 0; j < C2; ++j)
                for (size_t k = 0; k < C; ++k)
                    result(i, j) += (*this)(i, k) * other(k, j);
        return result;
    }

    mat<R, C, T>& operator*=(T scalar) {
        for (auto& v : m)
            v *= scalar;
        return *this;
    }

    mat<R, C, T>& operator=(const mat<R, C, T>& other) {
        if (this != &other)
            m = other.m;
        return *this;
    }

    mat<R, C, T>& operator*=(const mat<R, C, T>& other) {
        static_assert(R == C, "Matrix *= only valid for square matrices");
        *this = (*this) * other;
        return *this;
    }
    bool operator==(const mat<R, C, T>& other) const {
        return m == other.m;
    }
};

// Common aliases
using mat4 = mat<4, 4, float>;
using mat3 = mat<3, 3, float>;
using mat2 = mat<2, 2, float>;

// Column-major orthographic matrix (OpenGL-style)
inline mat4 orthographic(float left, float right, float bottom, float top, float near = -1.f, float far = 1.f) {
    mat4 m;
    m(0, 0) = 2.f / (right - left);
    m(1, 1) = 2.f / (top - bottom);
    m(2, 2) = -2.f / (far - near);
    m(3, 0) = -(right + left) / (right - left);
    m(3, 1) = -(top + bottom) / (top - bottom);
    m(3, 2) = -(far + near) / (far - near);
    m(3, 3) = 1.f;
    return m;
}

inline void translate(mat4& m, float x, float y, float z) {
    m(3, 0) += x;
    m(3, 1) += y;
    m(3, 2) += z;
}

inline void scale(mat4& m, float sx, float sy, float sz) {
    m(0, 0) *= sx;
    m(1, 1) *= sy;
    m(2, 2) *= sz;
}

} // namespace butil
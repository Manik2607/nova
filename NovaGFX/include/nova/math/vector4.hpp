#pragma once
#include "nova/math/vector3.hpp"

namespace nova {

template<Scalar T>
struct Vector4 {
    T x{0};
    T y{0};
    T z{0};
    T w{0};

    constexpr Vector4() noexcept = default;
    constexpr Vector4(T x, T y, T z, T w) noexcept : x(x), y(y), z(z), w(w) {}
    constexpr explicit Vector4(T s) noexcept : x(s), y(s), z(s), w(s) {}

    template<Scalar U>
    constexpr explicit Vector4(const Vector4<U>& o) noexcept 
        : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)), z(static_cast<T>(o.z)), w(static_cast<T>(o.w)) {}

    constexpr Vector4 operator+(const Vector4& o) const noexcept { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
    constexpr Vector4 operator-(const Vector4& o) const noexcept { return {x - o.x, y - o.y, z - o.z, w - o.w}; }
    constexpr Vector4 operator*(const Vector4& o) const noexcept { return {x * o.x, y * o.y, z * o.z, w * o.w}; }
    constexpr Vector4 operator/(const Vector4& o) const noexcept { return {x / o.x, y / o.y, z / o.z, w / o.w}; }
    constexpr Vector4 operator-() const noexcept { return {-x, -y, -z, -w}; }

    constexpr Vector4& operator+=(const Vector4& o) noexcept { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
    constexpr Vector4& operator-=(const Vector4& o) noexcept { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
    constexpr Vector4& operator*=(const Vector4& o) noexcept { x *= o.x; y *= o.y; z *= o.z; w *= o.w; return *this; }
    constexpr Vector4& operator/=(const Vector4& o) noexcept { x /= o.x; y /= o.y; z /= o.z; w /= o.w; return *this; }

    constexpr Vector4 operator+(T s) const noexcept { return {x + s, y + s, z + s, w + s}; }
    constexpr Vector4 operator-(T s) const noexcept { return {x - s, y - s, z - s, w - s}; }
    constexpr Vector4 operator*(T s) const noexcept { return {x * s, y * s, z * s, w * s}; }
    constexpr Vector4 operator/(T s) const noexcept { return {x / s, y / s, z / s, w / s}; }

    constexpr Vector4& operator+=(T s) noexcept { x += s; y += s; z += s; w += s; return *this; }
    constexpr Vector4& operator-=(T s) noexcept { x -= s; y -= s; z -= s; w -= s; return *this; }
    constexpr Vector4& operator*=(T s) noexcept { x *= s; y *= s; z *= s; w *= s; return *this; }
    constexpr Vector4& operator/=(T s) noexcept { x /= s; y /= s; z /= s; w /= s; return *this; }

    constexpr bool operator==(const Vector4& o) const noexcept = default;

    constexpr f32 length_squared() const noexcept { return static_cast<f32>(x * x + y * y + z * z + w * w); }
    f32 length() const noexcept { return std::sqrt(length_squared()); }
    
    Vector4<f32> normalized() const noexcept {
        f32 l = length();
        if (l == 0) return Vector4<f32>(0, 0, 0, 0);
        return Vector4<f32>(static_cast<f32>(x)/l, static_cast<f32>(y)/l, static_cast<f32>(z)/l, static_cast<f32>(w)/l);
    }
    
    constexpr T dot(const Vector4& o) const noexcept { return x * o.x + y * o.y + z * o.z + w * o.w; }
    
    constexpr Vector4<f32> lerp(const Vector4& to, f32 weight) const noexcept {
        return Vector4<f32>(
            static_cast<f32>(x) + (static_cast<f32>(to.x) - static_cast<f32>(x)) * weight,
            static_cast<f32>(y) + (static_cast<f32>(to.y) - static_cast<f32>(y)) * weight,
            static_cast<f32>(z) + (static_cast<f32>(to.z) - static_cast<f32>(z)) * weight,
            static_cast<f32>(w) + (static_cast<f32>(to.w) - static_cast<f32>(w)) * weight
        );
    }
    
    std::string to_string() const {
        return std::format("({}, {}, {}, {})", x, y, z, w);
    }
};

template<Scalar T>
constexpr Vector4<T> operator*(T s, const Vector4<T>& v) noexcept { return v * s; }

using Vector4f = Vector4<f32>;
using Vector4i = Vector4<i32>;

} // namespace nova

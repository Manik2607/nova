#pragma once
#include "nova/math/vector2.hpp"

namespace nova {

template<Scalar T>
struct Vector3 {
    T x{0};
    T y{0};
    T z{0};

    constexpr Vector3() noexcept = default;
    constexpr Vector3(T x, T y, T z) noexcept : x(x), y(y), z(z) {}
    constexpr explicit Vector3(T s) noexcept : x(s), y(s), z(s) {}

    template<Scalar U>
    constexpr explicit Vector3(const Vector3<U>& o) noexcept 
        : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)), z(static_cast<T>(o.z)) {}

    constexpr Vector3 operator+(const Vector3& o) const noexcept { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vector3 operator-(const Vector3& o) const noexcept { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vector3 operator*(const Vector3& o) const noexcept { return {x * o.x, y * o.y, z * o.z}; }
    constexpr Vector3 operator/(const Vector3& o) const noexcept { return {x / o.x, y / o.y, z / o.z}; }
    constexpr Vector3 operator-() const noexcept { return {-x, -y, -z}; }

    constexpr Vector3& operator+=(const Vector3& o) noexcept { x += o.x; y += o.y; z += o.z; return *this; }
    constexpr Vector3& operator-=(const Vector3& o) noexcept { x -= o.x; y -= o.y; z -= o.z; return *this; }
    constexpr Vector3& operator*=(const Vector3& o) noexcept { x *= o.x; y *= o.y; z *= o.z; return *this; }
    constexpr Vector3& operator/=(const Vector3& o) noexcept { x /= o.x; y /= o.y; z /= o.z; return *this; }

    constexpr Vector3 operator+(T s) const noexcept { return {x + s, y + s, z + s}; }
    constexpr Vector3 operator-(T s) const noexcept { return {x - s, y - s, z - s}; }
    constexpr Vector3 operator*(T s) const noexcept { return {x * s, y * s, z * s}; }
    constexpr Vector3 operator/(T s) const noexcept { return {x / s, y / s, z / s}; }

    constexpr Vector3& operator+=(T s) noexcept { x += s; y += s; z += s; return *this; }
    constexpr Vector3& operator-=(T s) noexcept { x -= s; y -= s; z -= s; return *this; }
    constexpr Vector3& operator*=(T s) noexcept { x *= s; y *= s; z *= s; return *this; }
    constexpr Vector3& operator/=(T s) noexcept { x /= s; y /= s; z /= s; return *this; }

    constexpr bool operator==(const Vector3& o) const noexcept = default;

    constexpr f32 length_squared() const noexcept { return static_cast<f32>(x * x + y * y + z * z); }
    f32 length() const noexcept { return std::sqrt(length_squared()); }
    
    Vector3<f32> normalized() const noexcept {
        f32 l = length();
        if (l == 0) return Vector3<f32>(0, 0, 0);
        return Vector3<f32>(static_cast<f32>(x) / l, static_cast<f32>(y) / l, static_cast<f32>(z) / l);
    }
    
    constexpr T dot(const Vector3& o) const noexcept { return x * o.x + y * o.y + z * o.z; }
    constexpr Vector3 cross(const Vector3& o) const noexcept { 
        return {
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        }; 
    }
    
    f32 distance_to(const Vector3& o) const noexcept { return (*this - o).length(); }
    
    constexpr Vector3<f32> lerp(const Vector3& to, f32 weight) const noexcept {
        return Vector3<f32>(
            static_cast<f32>(x) + (static_cast<f32>(to.x) - static_cast<f32>(x)) * weight,
            static_cast<f32>(y) + (static_cast<f32>(to.y) - static_cast<f32>(y)) * weight,
            static_cast<f32>(z) + (static_cast<f32>(to.z) - static_cast<f32>(z)) * weight
        );
    }
    
    constexpr Vector3 abs() const noexcept {
        return {x < 0 ? -x : x, y < 0 ? -y : y, z < 0 ? -z : z};
    }
    
    constexpr Vector3 clamp(const Vector3& min, const Vector3& max) const noexcept {
        return {
            x < min.x ? min.x : (x > max.x ? max.x : x),
            y < min.y ? min.y : (y > max.y ? max.y : y),
            z < min.z ? min.z : (z > max.z ? max.z : z)
        };
    }
    
    Vector3<f32> rotated(const Vector3<f32>& axis, f32 angle) const noexcept {
        Vector3<f32> v(static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(z));
        Vector3<f32> u = axis.normalized();
        f32 sine = std::sin(angle);
        f32 cosine = std::cos(angle);
        f32 dot_p = v.dot(u);
        Vector3<f32> cross_p = u.cross(v);
        
        return v * cosine + cross_p * sine + u * dot_p * (1.0f - cosine);
    }
    
    Vector3<f32> reflect(const Vector3<f32>& normal) const noexcept {
        Vector3<f32> v(static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(z));
        return v - normal * 2.0f * v.dot(normal);
    }
    
    Vector3<f32> slide(const Vector3<f32>& normal) const noexcept {
        Vector3<f32> v(static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(z));
        return v - normal * v.dot(normal);
    }
    
    std::string to_string() const {
        return std::format("({}, {}, {})", x, y, z);
    }

    static constexpr Vector3 ZERO()    { return {0, 0, 0}; }
    static constexpr Vector3 ONE()     { return {1, 1, 1}; }
    static constexpr Vector3 UP()      { return {0, 1, 0}; }
    static constexpr Vector3 DOWN()    { return {0, -1, 0}; }
    static constexpr Vector3 LEFT()    { return {-1, 0, 0}; }
    static constexpr Vector3 RIGHT()   { return {1, 0, 0}; }
    static constexpr Vector3 FORWARD() { return {0, 0, -1}; }
    static constexpr Vector3 BACK()    { return {0, 0, 1}; }
};

template<Scalar T>
constexpr Vector3<T> operator*(T s, const Vector3<T>& v) noexcept { return v * s; }

using Vector3f = Vector3<f32>;
using Vector3i = Vector3<i32>;

} // namespace nova

#pragma once
#include "nova/core.hpp"
#include <cmath>
#include <type_traits>
#include <format>
#include <string>

namespace nova {

template<typename T>
concept Scalar = std::is_arithmetic_v<T>;

template<Scalar T>
struct Vector2 {
    T x{0};
    T y{0};

    constexpr Vector2() noexcept = default;
    constexpr Vector2(T x, T y) noexcept : x(x), y(y) {}
    constexpr explicit Vector2(T scalar) noexcept : x(scalar), y(scalar) {}

    template<Scalar U>
    constexpr explicit Vector2(const Vector2<U>& o) noexcept 
        : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)) {}

    constexpr Vector2 operator+(const Vector2& o) const noexcept { return {x + o.x, y + o.y}; }
    constexpr Vector2 operator-(const Vector2& o) const noexcept { return {x - o.x, y - o.y}; }
    constexpr Vector2 operator*(const Vector2& o) const noexcept { return {x * o.x, y * o.y}; }
    constexpr Vector2 operator/(const Vector2& o) const noexcept { return {x / o.x, y / o.y}; }
    constexpr Vector2 operator-() const noexcept { return {-x, -y}; }

    constexpr Vector2& operator+=(const Vector2& o) noexcept { x += o.x; y += o.y; return *this; }
    constexpr Vector2& operator-=(const Vector2& o) noexcept { x -= o.x; y -= o.y; return *this; }
    constexpr Vector2& operator*=(const Vector2& o) noexcept { x *= o.x; y *= o.y; return *this; }
    constexpr Vector2& operator/=(const Vector2& o) noexcept { x /= o.x; y /= o.y; return *this; }

    constexpr Vector2 operator+(T s) const noexcept { return {x + s, y + s}; }
    constexpr Vector2 operator-(T s) const noexcept { return {x - s, y - s}; }
    constexpr Vector2 operator*(T s) const noexcept { return {x * s, y * s}; }
    constexpr Vector2 operator/(T s) const noexcept { return {x / s, y / s}; }

    constexpr Vector2& operator+=(T s) noexcept { x += s; y += s; return *this; }
    constexpr Vector2& operator-=(T s) noexcept { x -= s; y -= s; return *this; }
    constexpr Vector2& operator*=(T s) noexcept { x *= s; y *= s; return *this; }
    constexpr Vector2& operator/=(T s) noexcept { x /= s; y /= s; return *this; }
    constexpr bool operator==(const Vector2& o) const noexcept = default;

    constexpr f32 length_squared() const noexcept { return static_cast<f32>(x * x + y * y); }
    f32 length() const noexcept { return std::sqrt(length_squared()); }
    
    Vector2<f32> normalized() const noexcept {
        f32 l = length();
        if (l == 0) return Vector2<f32>(0, 0);
        return Vector2<f32>(static_cast<f32>(x) / l, static_cast<f32>(y) / l);
    }
    
    constexpr T dot(const Vector2& o) const noexcept { return x * o.x + y * o.y; }
    constexpr T cross(const Vector2& o) const noexcept { return x * o.y - y * o.x; }
    
    f32 distance_to(const Vector2& o) const noexcept { return (*this - o).length(); }
    f32 angle() const noexcept { return std::atan2(static_cast<f32>(y), static_cast<f32>(x)); }
    f32 angle_to(const Vector2& o) const noexcept { return std::atan2(static_cast<f32>(cross(o)), static_cast<f32>(dot(o))); }
    
    constexpr Vector2<f32> lerp(const Vector2& to, f32 weight) const noexcept {
        return Vector2<f32>(
            static_cast<f32>(x) + (static_cast<f32>(to.x) - static_cast<f32>(x)) * weight,
            static_cast<f32>(y) + (static_cast<f32>(to.y) - static_cast<f32>(y)) * weight
        );
    }
    
    constexpr Vector2 abs() const noexcept {
        return {x < 0 ? -x : x, y < 0 ? -y : y};
    }
    
    constexpr Vector2 clamp(const Vector2& min, const Vector2& max) const noexcept {
        return {
            x < min.x ? min.x : (x > max.x ? max.x : x),
            y < min.y ? min.y : (y > max.y ? max.y : y)
        };
    }
    
    Vector2<f32> rotated(f32 angle_rad) const noexcept {
        f32 sine = std::sin(angle_rad);
        f32 cosine = std::cos(angle_rad);
        f32 fx = static_cast<f32>(x);
        f32 fy = static_cast<f32>(y);
        return {fx * cosine - fy * sine, fx * sine + fy * cosine};
    }
    
    Vector2<f32> reflect(const Vector2<f32>& normal) const noexcept {
        Vector2<f32> v(static_cast<f32>(x), static_cast<f32>(y));
        return v - normal * 2.0f * v.dot(normal);
    }
    
    Vector2<f32> slide(const Vector2<f32>& normal) const noexcept {
        Vector2<f32> v(static_cast<f32>(x), static_cast<f32>(y));
        return v - normal * v.dot(normal);
    }
    
    Vector2 snapped(const Vector2& step) const noexcept {
        if constexpr (std::is_floating_point_v<T>) {
            return {
                step.x != 0 ? std::floor(x / step.x + T(0.5)) * step.x : x,
                step.y != 0 ? std::floor(y / step.y + T(0.5)) * step.y : y
            };
        } else {
            return {
                step.x != 0 ? (x / step.x) * step.x : x,
                step.y != 0 ? (y / step.y) * step.y : y
            };
        }
    }
    
    std::string to_string() const {
        return std::format("({}, {})", x, y);
    }

    static constexpr Vector2 ZERO()  { return {0, 0}; }
    static constexpr Vector2 ONE()   { return {1, 1}; }
    static constexpr Vector2 UP()    { return {0, -1}; }
    static constexpr Vector2 DOWN()  { return {0, 1}; }
    static constexpr Vector2 LEFT()  { return {-1, 0}; }
    static constexpr Vector2 RIGHT() { return {1, 0}; }
};

template<Scalar T>
constexpr Vector2<T> operator*(T s, const Vector2<T>& v) noexcept { return v * s; }

using Vector2f = Vector2<f32>;
using Vector2i = Vector2<i32>;

} // namespace nova

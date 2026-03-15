#pragma once
#include "nova/math/vector2.hpp"

namespace nova {

struct Transform2D {
    Vector2f x_axis{1, 0};
    Vector2f y_axis{0, 1};
    Vector2f origin{0, 0};

    constexpr Transform2D() noexcept = default;
    constexpr Transform2D(const Vector2f& x, const Vector2f& y, const Vector2f& o) noexcept
        : x_axis(x), y_axis(y), origin(o) {}

    static constexpr Transform2D identity() noexcept { return Transform2D(); }

    static constexpr Transform2D from_position(const Vector2f& pos) noexcept {
        return Transform2D({1, 0}, {0, 1}, pos);
    }

    static Transform2D from_rotation(f32 angle_rad) noexcept {
        f32 c = std::cos(angle_rad);
        f32 s = std::sin(angle_rad);
        return Transform2D({c, s}, {-s, c}, {0, 0});
    }

    static constexpr Transform2D from_scale(const Vector2f& scale) noexcept {
        return Transform2D({scale.x, 0}, {0, scale.y}, {0, 0});
    }

    constexpr Transform2D translated(const Vector2f& pos) const noexcept {
        return Transform2D(x_axis, y_axis, origin + x_axis * pos.x + y_axis * pos.y);
    }

    Transform2D rotated(f32 angle_rad) const noexcept {
        Transform2D r = from_rotation(angle_rad);
        return *this * r;
    }

    constexpr Transform2D scaled(const Vector2f& scale) const noexcept {
        return Transform2D(x_axis * scale.x, y_axis * scale.y, origin);
    }

    constexpr Transform2D inverse() const noexcept {
        f32 det = x_axis.x * y_axis.y - x_axis.y * y_axis.x;
        if (det == 0.0f) return identity();
        f32 inv_det = 1.0f / det;
        
        Transform2D inv;
        inv.x_axis = {y_axis.y * inv_det, -x_axis.y * inv_det};
        inv.y_axis = {-y_axis.x * inv_det, x_axis.x * inv_det};
        
        inv.origin = inv.xform_inv_no_translation(origin);
        inv.origin.x = -inv.origin.x;
        inv.origin.y = -inv.origin.y;
        return inv;
    }

    constexpr Vector2f xform(const Vector2f& v) const noexcept {
        return origin + x_axis * v.x + y_axis * v.y;
    }

    constexpr Vector2f xform_inv(const Vector2f& v) const noexcept {
        Vector2f v_sub = v - origin;
        return xform_inv_no_translation(v_sub);
    }

    constexpr Transform2D operator*(const Transform2D& o) const noexcept {
        return Transform2D(
            xform_no_translation(o.x_axis),
            xform_no_translation(o.y_axis),
            xform(o.origin)
        );
    }

private:
    constexpr Vector2f xform_no_translation(const Vector2f& v) const noexcept {
        return x_axis * v.x + y_axis * v.y;
    }

    constexpr Vector2f xform_inv_no_translation(const Vector2f& v) const noexcept {
        f32 det = x_axis.x * y_axis.y - x_axis.y * y_axis.x;
        if (det == 0.0f) return {0, 0};
        f32 inv_det = 1.0f / det;
        return {
            (v.x * y_axis.y - v.y * y_axis.x) * inv_det,
            (v.y * x_axis.x - v.x * x_axis.y) * inv_det
        };
    }
};

} // namespace nova

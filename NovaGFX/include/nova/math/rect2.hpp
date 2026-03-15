#pragma once
#include "nova/math/vector2.hpp"
#include <algorithm>

namespace nova {

template<Scalar T>
struct Rect2 {
    Vector2<T> position;
    Vector2<T> size;

    constexpr Rect2() noexcept = default;
    constexpr Rect2(Vector2<T> pos, Vector2<T> sz) noexcept : position(pos), size(sz) {}
    constexpr Rect2(T x, T y, T w, T h) noexcept : position(x, y), size(w, h) {}

    constexpr Vector2<T> end() const noexcept { return position + size; }
    constexpr Vector2<T> center() const noexcept { return position + size / T(2); }
    constexpr T area() const noexcept { return size.x * size.y; }

    constexpr bool contains(const Vector2<T>& v) const noexcept {
        return v.x >= position.x && v.x < position.x + size.x &&
               v.y >= position.y && v.y < position.y + size.y;
    }

    constexpr bool intersects(const Rect2& o) const noexcept {
        return !(position.x + size.x <= o.position.x ||
                 position.x >= o.position.x + o.size.x ||
                 position.y + size.y <= o.position.y ||
                 position.y >= o.position.y + o.size.y);
    }

    constexpr Rect2 intersection(const Rect2& o) const noexcept {
        if (!intersects(o)) return Rect2{};
        Vector2<T> i_pos(
            std::max(position.x, o.position.x),
            std::max(position.y, o.position.y)
        );
        Vector2<T> i_end(
            std::min(position.x + size.x, o.position.x + o.size.x),
            std::min(position.y + size.y, o.position.y + o.size.y)
        );
        return Rect2(i_pos, i_end - i_pos);
    }

    constexpr Rect2 merged(const Rect2& o) const noexcept {
        Vector2<T> m_pos(
            std::min(position.x, o.position.x),
            std::min(position.y, o.position.y)
        );
        Vector2<T> m_end(
            std::max(position.x + size.x, o.position.x + o.size.x),
            std::max(position.y + size.y, o.position.y + o.size.y)
        );
        return Rect2(m_pos, m_end - m_pos);
    }

    constexpr Rect2 expanded_to(const Vector2<T>& v) const noexcept {
        Vector2<T> origin(
            std::min(position.x, v.x),
            std::min(position.y, v.y)
        );
        Vector2<T> target(
            std::max(position.x + size.x, v.x),
            std::max(position.y + size.y, v.y)
        );
        return Rect2(origin, target - origin);
    }

    constexpr Rect2 grow(T amount) const noexcept {
        return Rect2(
            position.x - amount, position.y - amount,
            size.x + amount * 2, size.y + amount * 2
        );
    }

    constexpr bool has_area() const noexcept {
        return size.x > 0 && size.y > 0;
    }
    
    std::string to_string() const {
        return std::format("[pos: {}, size: {}]", position.to_string(), size.to_string());
    }
};

using Rect2f = Rect2<f32>;
using Rect2i = Rect2<i32>;

} // namespace nova

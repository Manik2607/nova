#pragma once
#include "nova/core.hpp"
#include <string_view>

namespace nova {

struct Color {
    f32 r{0}, g{0}, b{0}, a{1};

    constexpr Color() noexcept = default;
    constexpr Color(f32 r, f32 g, f32 b, f32 a = 1.0f) noexcept : r(r), g(g), b(b), a(a) {}

    static constexpr Color RED()         { return {1, 0, 0, 1}; }
    static constexpr Color GREEN()       { return {0, 1, 0, 1}; }
    static constexpr Color BLUE()        { return {0, 0, 1, 1}; }
    static constexpr Color WHITE()       { return {1, 1, 1, 1}; }
    static constexpr Color BLACK()       { return {0, 0, 0, 1}; }
    static constexpr Color TRANSPARENT() { return {0, 0, 0, 0}; }
    static constexpr Color YELLOW()      { return {1, 1, 0, 1}; }
    static constexpr Color CYAN()        { return {0, 1, 1, 1}; }
    static constexpr Color MAGENTA()     { return {1, 0, 1, 1}; }
    static constexpr Color ORANGE()      { return {1, 0.5f, 0, 1}; }
    static constexpr Color GRAY()        { return {0.5f, 0.5f, 0.5f, 1}; }

    constexpr u32 to_u32() const noexcept {
        return (static_cast<u32>(r * 255.0f) << 24) |
               (static_cast<u32>(g * 255.0f) << 16) |
               (static_cast<u32>(b * 255.0f) << 8) |
               (static_cast<u32>(a * 255.0f));
    }

    static constexpr Color from_u32(u32 c) noexcept {
        return {
            static_cast<f32>((c >> 24) & 0xFF) / 255.0f,
            static_cast<f32>((c >> 16) & 0xFF) / 255.0f,
            static_cast<f32>((c >> 8)  & 0xFF) / 255.0f,
            static_cast<f32>(c & 0xFF) / 255.0f
        };
    }

    static constexpr Color from_hex(std::string_view hex) noexcept {
        if (hex.empty()) return BLACK();
        usize start = (hex[0] == '#') ? 1 : 0;
        usize len = hex.length() - start;
        u32 val = 0;
        for (usize i = 0; i < len; ++i) {
            char c = hex[start + i];
            val <<= 4;
            if (c >= '0' && c <= '9') val |= (c - '0');
            else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
        }
        if (len == 6) {
            val = (val << 8) | 0xFF; // add alpha
        } else if (len == 3) {
            u32 rr = (val >> 8) & 0xF;
            u32 gg = (val >> 4) & 0xF;
            u32 bb = val & 0xF;
            val = (rr << 28) | (rr << 24) | (gg << 20) | (gg << 16) | (bb << 12) | (bb << 8) | 0xFF;
        }
        return from_u32(val);
    }

    constexpr Color lerp(const Color& o, f32 t) const noexcept {
        return {
            r + (o.r - r) * t,
            g + (o.g - g) * t,
            b + (o.b - b) * t,
            a + (o.a - a) * t
        };
    }

    constexpr Color darkened(f32 amount) const noexcept {
        return { r * (1.0f - amount), g * (1.0f - amount), b * (1.0f - amount), a };
    }

    constexpr Color lightened(f32 amount) const noexcept {
        return { r + (1.0f - r) * amount, g + (1.0f - g) * amount, b + (1.0f - b) * amount, a };
    }

    constexpr Color operator*(f32 s) const noexcept {
        return {r, g, b, a * s};
    }
};

} // namespace nova

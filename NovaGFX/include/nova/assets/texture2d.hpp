#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include <string_view>

namespace nova {

class Texture2D {
public:
    enum class FilterMode {
        NEAREST,
        LINEAR
    };

    enum class WrapMode {
        CLAMP,
        REPEAT,
        MIRRORED_REPEAT
    };

    Texture2D(std::string_view path, FilterMode filter = FilterMode::LINEAR, WrapMode wrap = WrapMode::REPEAT);
    Texture2D(const u8* pixels, i32 width, i32 height, i32 channels, FilterMode filter = FilterMode::LINEAR, WrapMode wrap = WrapMode::REPEAT);
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    void bind(u32 slot = 0) const;
    void unbind() const;

    i32 get_width() const { return m_width; }
    i32 get_height() const { return m_height; }
    Vector2i get_size() const { return {m_width, m_height}; }
    u32 handle() const { return m_handle; }

private:
    u32 m_handle{0};
    i32 m_width{0};
    i32 m_height{0};
    i32 m_channels{0};

    void setup_gl(const u8* pixels, FilterMode filter, WrapMode wrap);
};

} // namespace nova

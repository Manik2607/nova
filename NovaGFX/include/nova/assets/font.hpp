#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/assets/texture2d.hpp"
#include <string_view>
#include <vector>
#include <memory>
#include <map>

namespace nova {

/**
 * Font — A TrueType font resource.
 * Handles loading .ttf files and generating a texture atlas for rendering.
 */
class Font {
public:
    struct Glyph {
        Vector2f size;
        Vector2f bearing;
        Vector2f uv_min;
        Vector2f uv_max;
        f32 advance;
    };

    Font(std::string_view path, f32 size = 32.0f);
    Font(const u8* data, usize size_bytes, f32 size = 32.0f);
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    const Glyph* get_glyph(char c) const;
    f32 get_size() const { return m_font_size; }
    const Texture2D& get_atlas() const { return *m_atlas; }

    f32 measure_text(std::string_view text) const;

private:
    void init(const u8* data, usize size_bytes, f32 size);

    f32 m_font_size;
    std::unique_ptr<Texture2D> m_atlas;
    std::map<char, Glyph> m_glyphs;
};

} // namespace nova

/**
 * TextRenderer — Engine-level vector font text rendering.
 *
 * Draws text using line-segment vector glyphs through Renderer2D.
 * Resolution-independent, neon-aesthetic friendly.
 *
 * API Surface:
 *   TextRenderer text;
 *   text.draw_text(renderer, "HELLO", {100, 50}, 24.0f, Color::WHITE());
 *   text.draw_text(renderer, "Score: 1234", pos, size, color, 2.0f); // custom thickness
 *   f32 w = text.measure_text("HELLO", 24.0f); // get text width
 */
#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/color.hpp"
#include "nova/renderer/renderer2d.hpp"
#include "nova/assets/font.hpp"
#include <string_view>
#include <vector>
#include <array>
#include <memory>

namespace nova {

class TextRenderer {
public:
    TextRenderer();

    /// Draw a string at the given position with font size and color.
    void draw_text(Renderer2D& renderer, std::string_view text,
                   Vector2f position, f32 size, Color color,
                   f32 thickness = 1.0f);

    /// Draw text using a specific font.
    void draw_text_with_font(Renderer2D& renderer, const Font& font, std::string_view text,
                            Vector2f position, f32 size, Color color);

    /// Measure the width of a string at the given font size.
    f32 measure_text(std::string_view text, f32 size) const;

    /// Measure the width of a string using a specific font.
    f32 measure_text_with_font(const Font& font, std::string_view text, f32 size) const;

    /// Get the default font.
    Font& get_default_font() { return *m_default_font; }

    /// Character spacing multiplier (default 1.0).
    f32 spacing = 1.0f;

private:
    std::unique_ptr<Font> m_default_font;

    // Legacy vector font data (unused in new system but kept for reference if needed)
    struct GlyphSegment {
        f32 x1, y1, x2, y2;
    };

    struct Glyph {
        std::vector<GlyphSegment> segments;
        f32 width = 0.6f;
    };

    std::array<Glyph, 128> m_glyphs;
    void init_glyphs();
};

} // namespace nova

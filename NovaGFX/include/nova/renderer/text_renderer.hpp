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
#include <string_view>
#include <vector>
#include <array>

namespace nova {

class TextRenderer {
public:
    TextRenderer();

    /// Draw a string at the given position with font size and color.
    void draw_text(Renderer2D& renderer, std::string_view text,
                   Vector2f position, f32 size, Color color,
                   f32 thickness = 1.5f);

    /// Measure the width of a string at the given font size.
    f32 measure_text(std::string_view text, f32 size) const;

    /// Character spacing multiplier (default 1.0).
    f32 spacing = 1.0f;

private:
    // Each glyph is a set of line segments in a 0-1 normalized space
    struct GlyphSegment {
        f32 x1, y1, x2, y2;
    };

    struct Glyph {
        std::vector<GlyphSegment> segments;
        f32 width = 0.6f; // normalized width
    };

    std::array<Glyph, 128> m_glyphs;
    void init_glyphs();

    void draw_glyph(Renderer2D& renderer, char c, Vector2f pos,
                    f32 size, Color color, f32 thickness);
};

} // namespace nova

#include "nova/renderer/text_renderer.hpp"
#include <cctype>

namespace nova {

TextRenderer::TextRenderer() {
    init_glyphs();
}

void TextRenderer::init_glyphs() {
    // Define vector font glyphs as line segments in normalized [0,1] space.
    // Each character fits in a cell approximately 0.6 wide × 1.0 tall.
    auto& g = m_glyphs;
    
    // Helper to define segments
    auto seg = [](f32 x1, f32 y1, f32 x2, f32 y2) -> GlyphSegment {
        return {x1, y1, x2, y2};
    };

    // Digits 0-9
    g['0'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,1), seg(0.5f,1,0.1f,1), seg(0.1f,1,0.1f,0), seg(0.1f,1,0.5f,0)};
    g['1'].segments = {seg(0.3f,0,0.3f,1), seg(0.1f,0.2f,0.3f,0), seg(0.1f,1,0.5f,1)};
    g['2'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,0.5f), seg(0.5f,0.5f,0.1f,0.5f), seg(0.1f,0.5f,0.1f,1), seg(0.1f,1,0.5f,1)};
    g['3'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,1), seg(0.5f,1,0.1f,1), seg(0.1f,0.5f,0.5f,0.5f)};
    g['4'].segments = {seg(0.1f,0,0.1f,0.5f), seg(0.1f,0.5f,0.5f,0.5f), seg(0.5f,0,0.5f,1)};
    g['5'].segments = {seg(0.5f,0,0.1f,0), seg(0.1f,0,0.1f,0.5f), seg(0.1f,0.5f,0.5f,0.5f), seg(0.5f,0.5f,0.5f,1), seg(0.5f,1,0.1f,1)};
    g['6'].segments = {seg(0.5f,0,0.1f,0), seg(0.1f,0,0.1f,1), seg(0.1f,1,0.5f,1), seg(0.5f,1,0.5f,0.5f), seg(0.5f,0.5f,0.1f,0.5f)};
    g['7'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.3f,1)};
    g['8'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,1), seg(0.5f,1,0.1f,1), seg(0.1f,1,0.1f,0), seg(0.1f,0.5f,0.5f,0.5f)};
    g['9'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,1), seg(0.5f,1,0.1f,1), seg(0.1f,0,0.1f,0.5f), seg(0.1f,0.5f,0.5f,0.5f)};

    // Letters A-Z
    g['A'].segments = {seg(0,1,0.3f,0), seg(0.3f,0,0.6f,1), seg(0.1f,0.55f,0.5f,0.55f)};
    g['B'].segments = {seg(0.1f,0,0.1f,1), seg(0.1f,0,0.4f,0), seg(0.4f,0,0.5f,0.12f), seg(0.5f,0.12f,0.5f,0.38f), seg(0.5f,0.38f,0.4f,0.5f), seg(0.1f,0.5f,0.4f,0.5f), seg(0.4f,0.5f,0.5f,0.62f), seg(0.5f,0.62f,0.5f,0.88f), seg(0.5f,0.88f,0.4f,1), seg(0.4f,1,0.1f,1)};
    g['C'].segments = {seg(0.5f,0,0.1f,0), seg(0.1f,0,0.1f,1), seg(0.1f,1,0.5f,1)};
    g['D'].segments = {seg(0.1f,0,0.1f,1), seg(0.1f,0,0.4f,0), seg(0.4f,0,0.5f,0.2f), seg(0.5f,0.2f,0.5f,0.8f), seg(0.5f,0.8f,0.4f,1), seg(0.4f,1,0.1f,1)};
    g['E'].segments = {seg(0.5f,0,0.1f,0), seg(0.1f,0,0.1f,1), seg(0.1f,1,0.5f,1), seg(0.1f,0.5f,0.4f,0.5f)};
    g['F'].segments = {seg(0.5f,0,0.1f,0), seg(0.1f,0,0.1f,1), seg(0.1f,0.5f,0.4f,0.5f)};
    g['G'].segments = {seg(0.5f,0,0.1f,0), seg(0.1f,0,0.1f,1), seg(0.1f,1,0.5f,1), seg(0.5f,1,0.5f,0.5f), seg(0.5f,0.5f,0.3f,0.5f)};
    g['H'].segments = {seg(0.1f,0,0.1f,1), seg(0.5f,0,0.5f,1), seg(0.1f,0.5f,0.5f,0.5f)};
    g['I'].segments = {seg(0.15f,0,0.45f,0), seg(0.3f,0,0.3f,1), seg(0.15f,1,0.45f,1)};
    g['J'].segments = {seg(0.2f,0,0.5f,0), seg(0.4f,0,0.4f,1), seg(0.4f,1,0.2f,1), seg(0.2f,1,0.1f,0.8f)};
    g['K'].segments = {seg(0.1f,0,0.1f,1), seg(0.5f,0,0.1f,0.5f), seg(0.1f,0.5f,0.5f,1)};
    g['L'].segments = {seg(0.1f,0,0.1f,1), seg(0.1f,1,0.5f,1)};
    g['M'].segments = {seg(0.1f,1,0.1f,0), seg(0.1f,0,0.3f,0.4f), seg(0.3f,0.4f,0.5f,0), seg(0.5f,0,0.5f,1)};
    g['N'].segments = {seg(0.1f,1,0.1f,0), seg(0.1f,0,0.5f,1), seg(0.5f,1,0.5f,0)};
    g['O'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,1), seg(0.5f,1,0.1f,1), seg(0.1f,1,0.1f,0)};
    g['P'].segments = {seg(0.1f,0,0.1f,1), seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,0.5f), seg(0.5f,0.5f,0.1f,0.5f)};
    g['Q'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,0.8f), seg(0.5f,0.8f,0.1f,1.0f), seg(0.1f,1,0.1f,0), seg(0.35f,0.75f,0.55f,1.05f)};
    g['R'].segments = {seg(0.1f,0,0.1f,1), seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,0.5f), seg(0.5f,0.5f,0.1f,0.5f), seg(0.3f,0.5f,0.5f,1)};
    g['S'].segments = {seg(0.5f,0,0.1f,0), seg(0.1f,0,0.1f,0.5f), seg(0.1f,0.5f,0.5f,0.5f), seg(0.5f,0.5f,0.5f,1), seg(0.5f,1,0.1f,1)};
    g['T'].segments = {seg(0.0f,0,0.6f,0), seg(0.3f,0,0.3f,1)};
    g['U'].segments = {seg(0.1f,0,0.1f,1), seg(0.1f,1,0.5f,1), seg(0.5f,1,0.5f,0)};
    g['V'].segments = {seg(0.0f,0,0.3f,1), seg(0.3f,1,0.6f,0)};
    g['W'].segments = {seg(0.0f,0,0.1f,1), seg(0.1f,1,0.3f,0.6f), seg(0.3f,0.6f,0.5f,1), seg(0.5f,1,0.6f,0)};
    g['X'].segments = {seg(0.1f,0,0.5f,1), seg(0.5f,0,0.1f,1)};
    g['Y'].segments = {seg(0.0f,0,0.3f,0.5f), seg(0.6f,0,0.3f,0.5f), seg(0.3f,0.5f,0.3f,1)};
    g['Z'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.1f,1), seg(0.1f,1,0.5f,1)};

    // Lowercase (map to uppercase)
    for (int c = 'a'; c <= 'z'; c++) {
        g[static_cast<usize>(c)] = g[static_cast<usize>(c - 32)];
    }

    // Punctuation / symbols
    g['.'].segments = {seg(0.25f,0.9f,0.35f,0.9f), seg(0.35f,0.9f,0.35f,1), seg(0.35f,1,0.25f,1), seg(0.25f,1,0.25f,0.9f)};
    g['.'].width = 0.35f;
    g[','].segments = {seg(0.3f,0.85f,0.2f,1.05f)};
    g[','].width = 0.35f;
    g[':'].segments = {seg(0.25f,0.2f,0.35f,0.2f), seg(0.35f,0.2f,0.35f,0.3f), seg(0.35f,0.3f,0.25f,0.3f), seg(0.25f,0.3f,0.25f,0.2f), seg(0.25f,0.7f,0.35f,0.7f), seg(0.35f,0.7f,0.35f,0.8f), seg(0.35f,0.8f,0.25f,0.8f), seg(0.25f,0.8f,0.25f,0.7f)};
    g[':'].width = 0.35f;
    g['!'].segments = {seg(0.3f,0,0.3f,0.7f), seg(0.25f,0.9f,0.35f,0.9f), seg(0.35f,0.9f,0.35f,1), seg(0.35f,1,0.25f,1), seg(0.25f,1,0.25f,0.9f)};
    g['!'].width = 0.35f;
    g['?'].segments = {seg(0.1f,0,0.5f,0), seg(0.5f,0,0.5f,0.4f), seg(0.5f,0.4f,0.3f,0.5f), seg(0.3f,0.5f,0.3f,0.7f), seg(0.25f,0.9f,0.35f,0.9f), seg(0.35f,0.9f,0.35f,1), seg(0.35f,1,0.25f,1), seg(0.25f,1,0.25f,0.9f)};
    g['-'].segments = {seg(0.1f,0.5f,0.5f,0.5f)};
    g['+'].segments = {seg(0.1f,0.5f,0.5f,0.5f), seg(0.3f,0.3f,0.3f,0.7f)};
    g['/'].segments = {seg(0.5f,0,0.1f,1)};
    g['('].segments = {seg(0.35f,0,0.2f,0.25f), seg(0.2f,0.25f,0.2f,0.75f), seg(0.2f,0.75f,0.35f,1)};
    g['('].width = 0.4f;
    g[')'].segments = {seg(0.25f,0,0.4f,0.25f), seg(0.4f,0.25f,0.4f,0.75f), seg(0.4f,0.75f,0.25f,1)};
    g[')'].width = 0.4f;
    g['%'].segments = {seg(0.5f,0,0.1f,1), seg(0.1f,0.1f,0.2f,0.1f), seg(0.2f,0.1f,0.2f,0.25f), seg(0.2f,0.25f,0.1f,0.25f), seg(0.1f,0.25f,0.1f,0.1f), seg(0.4f,0.75f,0.5f,0.75f), seg(0.5f,0.75f,0.5f,0.9f), seg(0.5f,0.9f,0.4f,0.9f), seg(0.4f,0.9f,0.4f,0.75f)};
    g['x'].segments = g['X'].segments; // multiplication sign
    g['*'].segments = {seg(0.1f,0.3f,0.5f,0.7f), seg(0.5f,0.3f,0.1f,0.7f), seg(0.3f,0.2f,0.3f,0.8f)};
    g[' '].segments = {};
    g[' '].width = 0.35f;

    // Set default widths for glyphs that don't have custom widths
    for (auto& glyph : g) {
        if (glyph.width == 0.0f && !glyph.segments.empty()) {
            glyph.width = 0.6f;
        }
    }
}

void TextRenderer::draw_text(Renderer2D& renderer, std::string_view text,
                              Vector2f position, f32 size, Color color,
                              f32 thickness) {
    f32 cursor_x = position.x;
    f32 line_thickness = thickness * (size / 20.0f);
    if (line_thickness < 1.0f) line_thickness = 1.0f;

    for (char c : text) {
        if (c == '\n') {
            cursor_x = position.x;
            position.y += size * 1.2f;
            continue;
        }
        draw_glyph(renderer, c, {cursor_x, position.y}, size, color, line_thickness);
        usize idx = static_cast<usize>(static_cast<unsigned char>(c));
        f32 w = (idx < 128 && !m_glyphs[idx].segments.empty()) ? m_glyphs[idx].width : 0.6f;
        cursor_x += w * size * spacing + size * 0.08f; // glyph width + gap
    }
}

f32 TextRenderer::measure_text(std::string_view text, f32 size) const {
    f32 width = 0.0f;
    for (char c : text) {
        usize idx = static_cast<usize>(static_cast<unsigned char>(c));
        f32 w = (idx < 128 && !m_glyphs[idx].segments.empty()) ? m_glyphs[idx].width : 0.6f;
        width += w * size * spacing + size * 0.08f;
    }
    return width;
}

void TextRenderer::draw_glyph(Renderer2D& renderer, char c, Vector2f pos,
                                f32 size, Color color, f32 thickness) {
    usize idx = static_cast<usize>(static_cast<unsigned char>(c));
    if (idx >= 128) return;
    const auto& glyph = m_glyphs[idx];

    for (const auto& seg : glyph.segments) {
        Vector2f a = pos + Vector2f(seg.x1 * size, seg.y1 * size);
        Vector2f b = pos + Vector2f(seg.x2 * size, seg.y2 * size);
        renderer.draw_line(a, b, color, thickness);
    }
}

} // namespace nova

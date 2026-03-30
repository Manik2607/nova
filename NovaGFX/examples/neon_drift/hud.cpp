#include "hud.hpp"
#include <cmath>
#include <algorithm>

// Simple int-to-string without std::to_string (to keep things lean)
static void int_to_buf(char* buf, i32 val) {
    if (val < 0) { *buf++ = '-'; val = -val; }
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[16]; int i = 0;
    while (val > 0) { tmp[i++] = '0' + (val % 10); val /= 10; }
    for (int j = i - 1; j >= 0; j--) *buf++ = tmp[j];
    *buf = '\0';
}

void HUD::draw(Renderer2D& renderer, TextRenderer& text,
               Vector2f viewport, f32 speed, const DriftScorer& scorer, f32 fps) {
    f32 margin = 20.0f;
    char buf[64];

    // ── Title (top-left, small) ──
    text.draw_text(renderer, "NEON DRIFT",
                   {margin, margin}, 14.0f,
                   Color(0.5f, 0.0f, 1.0f, 0.5f), 1.0f);

    // ── FPS (top-right, subtle) ──
    {
        int_to_buf(buf, static_cast<i32>(fps));
        char fps_label[32] = "FPS ";
        char* p = fps_label + 4;
        char* s = buf; while (*s) *p++ = *s++;
        *p = '\0';

        f32 tw = text.measure_text(fps_label, 10.0f);
        Color fps_color = fps > 30.0f ? Color(0.0f, 1.0f, 0.5f, 0.5f)
                                      : Color(1.0f, 0.2f, 0.2f, 0.7f);
        text.draw_text(renderer, fps_label,
                       {viewport.x - tw - margin, margin}, 10.0f, fps_color);
    }

    // ── Speed (bottom-left) ──
    {
        int_to_buf(buf, static_cast<i32>(speed));
        char speed_label[32];
        char* p = speed_label;
        char* s = buf; while (*s) *p++ = *s++;
        *p++ = ' '; *p++ = 'K'; *p++ = 'M'; *p++ = '/'; *p++ = 'H'; *p = '\0';

        f32 speed_ratio = std::min(speed / 800.0f, 1.0f);
        Color speed_color = Color(0.0f, 1.0f, 0.8f, 1.0f).lerp(
            Color(1.0f, 0.2f, 0.4f, 1.0f), speed_ratio);

        text.draw_text(renderer, speed_label,
                       {margin, viewport.y - margin - 32.0f}, 28.0f, speed_color, 2.5f);

        // Speed label
        text.draw_text(renderer, "SPEED",
                       {margin, viewport.y - margin - 44.0f}, 10.0f,
                       Color(0.4f, 0.4f, 0.5f, 0.6f));

        // Speed bar underneath
        f32 bar_w = 220.0f;
        f32 bar_h = 4.0f;
        f32 bar_y = viewport.y - margin - 6.0f;
        renderer.draw_rect({margin, bar_y}, {bar_w, bar_h}, Color(0.1f, 0.1f, 0.15f, 0.6f));
        renderer.draw_rect({margin, bar_y}, {bar_w * speed_ratio, bar_h}, speed_color);
    }

    // ── Score (top-center) ──
    {
        i32 total = scorer.get_total_score();
        int_to_buf(buf, total);

        f32 tw = text.measure_text(buf, 22.0f);
        f32 cx = viewport.x * 0.5f - tw * 0.5f;

        text.draw_text(renderer, "SCORE",
                       {cx + tw * 0.5f - text.measure_text("SCORE", 9.0f) * 0.5f, margin},
                       9.0f, Color(0.5f, 0.4f, 0.6f, 0.6f));
        text.draw_text(renderer, buf,
                       {cx, margin + 14.0f}, 22.0f,
                       Color(0.9f, 0.85f, 1.0f, 1.0f), 2.0f);
    }

    // ── Drift combo indicator (center) ──
    if (scorer.is_in_drift()) {
        f32 cx = viewport.x * 0.5f;
        f32 cy = 80.0f;
        i32 combo = scorer.get_current_combo();
        f32 drift_score = scorer.get_current_drift_score();

        // "DRIFT!" text
        Color drift_color(1.0f, 0.3f, 0.0f, 1.0f);
        f32 dw = text.measure_text("DRIFT!", 20.0f);
        text.draw_text(renderer, "DRIFT!",
                       {cx - dw * 0.5f, cy}, 20.0f, drift_color, 2.5f);

        // Combo multiplier
        if (combo > 1) {
            char combo_buf[16] = "x";
            int_to_buf(combo_buf + 1, combo);
            f32 cw = text.measure_text(combo_buf, 16.0f);
            text.draw_text(renderer, combo_buf,
                           {cx - cw * 0.5f, cy + 26.0f}, 16.0f,
                           Color(1.0f, 0.85f, 0.0f, 1.0f), 2.0f);
        }

        // Drift score bar
        f32 bar_w = std::min(drift_score * 3.0f, 300.0f);
        renderer.draw_rect({cx - bar_w * 0.5f, cy + 48.0f}, {bar_w, 3.0f},
                          Color(1.0f, 0.4f, 0.0f, 0.8f));
    }
}

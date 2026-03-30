/**
 * DecalSystem — Persistent tire marks / decals rendered as a circular buffer.
 *
 * API Surface:
 *   DecalSystem decals(512);   // max marks
 *   decals.add_mark(pos_a, pos_b, width, color);
 *   decals.update(delta);      // fades alpha over time
 *   decals.draw(renderer);     // renders all active marks
 */
#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/color.hpp"
#include "nova/renderer/renderer2d.hpp"

namespace nova {

class DecalSystem {
public:
    explicit DecalSystem(usize max_decals = 512);

    /// Add a line mark (e.g. tire mark from point a to point b).
    void add_mark(Vector2f a, Vector2f b, f32 width, Color color, f32 lifetime = 5.0f);

    /// Update all marks (fades alpha).
    void update(f32 delta);

    /// Draw all visible marks.
    void draw(Renderer2D& renderer);

    void clear();

private:
    struct Decal {
        Vector2f a, b;
        f32 width;
        Color color;
        f32 lifetime;
        f32 max_lifetime;
        bool active = false;
    };

    std::vector<Decal> m_buffer;
    usize m_head = 0; // circular write index
};

} // namespace nova

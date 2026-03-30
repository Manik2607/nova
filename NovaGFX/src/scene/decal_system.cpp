#include "nova/scene/decal_system.hpp"

namespace nova {

DecalSystem::DecalSystem(usize max_decals)
    : m_buffer(max_decals)
{}

void DecalSystem::add_mark(Vector2f a, Vector2f b, f32 width, Color color, f32 lifetime) {
    Decal& d = m_buffer[m_head];
    d.a = a;
    d.b = b;
    d.width = width;
    d.color = color;
    d.lifetime = lifetime;
    d.max_lifetime = lifetime;
    d.active = true;

    m_head = (m_head + 1) % m_buffer.size();
}

void DecalSystem::update(f32 delta) {
    for (auto& d : m_buffer) {
        if (!d.active) continue;
        d.lifetime -= delta;
        if (d.lifetime <= 0.0f) {
            d.active = false;
        }
    }
}

void DecalSystem::draw(Renderer2D& renderer) {
    for (const auto& d : m_buffer) {
        if (!d.active) continue;

        f32 alpha = d.lifetime / d.max_lifetime;
        Color c = d.color;
        c.a *= alpha;

        renderer.draw_line(d.a, d.b, c, d.width);
    }
}

void DecalSystem::clear() {
    for (auto& d : m_buffer) {
        d.active = false;
    }
    m_head = 0;
}

} // namespace nova

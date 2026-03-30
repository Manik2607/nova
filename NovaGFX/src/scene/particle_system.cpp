#include "nova/scene/particle_system.hpp"
#include <cmath>
#include <random>
#include <algorithm>

namespace nova {

static std::mt19937 s_rng{std::random_device{}()};
static std::uniform_real_distribution<f32> s_unit(0.0f, 1.0f);
static std::uniform_real_distribution<f32> s_sym(-1.0f, 1.0f);

static f32 rand_range(f32 a, f32 b) { return a + (b - a) * s_unit(s_rng); }

ParticleSystem::ParticleSystem(usize max_particles)
    : m_pool(max_particles)
{}

usize ParticleSystem::add_emitter(const ParticleEmitter& emitter) {
    m_emitters.push_back(emitter);
    return m_emitters.size() - 1;
}

void ParticleSystem::spawn_particle(const ParticleEmitter& emitter) {
    // Find a free slot starting from hint
    usize start = m_next_free;
    usize pool_size = m_pool.size();
    for (usize i = 0; i < pool_size; i++) {
        usize idx = (start + i) % pool_size;
        if (!m_pool[idx].alive) {
            Particle& p = m_pool[idx];
            p.alive = true;
            p.position = emitter.position;

            // Random direction within spread
            f32 angle = emitter.rotation + s_sym(s_rng) * emitter.spread_angle;
            f32 speed = rand_range(emitter.velocity_min, emitter.velocity_max);
            p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};

            p.max_lifetime = rand_range(emitter.lifetime_min, emitter.lifetime_max);
            p.lifetime = p.max_lifetime;

            p.size_start = emitter.size_start;
            p.size_end = emitter.size_end;
            p.color_start = emitter.color_start;
            p.color_end = emitter.color_end;
            p.gravity_scale = emitter.gravity_scale;
            p.drag = emitter.drag;

            m_alive_count++;
            m_next_free = (idx + 1) % pool_size;
            return;
        }
    }
    // Pool full — silently drop
}

void ParticleSystem::emit(usize emitter_index, i32 count) {
    if (emitter_index >= m_emitters.size()) return;
    const auto& e = m_emitters[emitter_index];
    for (i32 i = 0; i < count; i++) {
        spawn_particle(e);
    }
}

void ParticleSystem::update(f32 delta) {
    // Handle continuous emitters
    for (auto& e : m_emitters) {
        if (!e.active) continue;
        if (e.mode == EmitterMode::CONTINUOUS) {
            e._accumulator += e.rate * delta;
            while (e._accumulator >= 1.0f) {
                spawn_particle(e);
                e._accumulator -= 1.0f;
            }
        }
    }

    // Update all alive particles
    m_alive_count = 0;
    for (auto& p : m_pool) {
        if (!p.alive) continue;

        p.lifetime -= delta;
        if (p.lifetime <= 0.0f) {
            p.alive = false;
            continue;
        }

        // Apply drag
        if (p.drag > 0.0f) {
            f32 drag_factor = std::max(0.0f, 1.0f - p.drag * delta);
            p.velocity = p.velocity * drag_factor;
        }

        // Apply gravity (top-down, so default is no gravity)
        p.velocity.y += p.gravity_scale * 980.0f * delta;

        p.position = p.position + p.velocity * delta;
        m_alive_count++;
    }
}

void ParticleSystem::draw(Renderer2D& renderer) {
    for (const auto& p : m_pool) {
        if (!p.alive) continue;

        f32 t = 1.0f - (p.lifetime / p.max_lifetime); // 0→1 over lifetime
        f32 size = p.size_start + (p.size_end - p.size_start) * t;
        Color color = p.color_start.lerp(p.color_end, t);

        renderer.draw_rect(
            p.position - Vector2f(size * 0.5f, size * 0.5f),
            {size, size},
            color
        );
    }
}

} // namespace nova

/**
 * ParticleSystem — Pooled particle emitter rendered through Renderer2D.
 *
 * API Surface:
 *   ParticleSystem ps(2048);      // max particles
 *   ParticleEmitter& e = ps.add_emitter();
 *   e.position = car_pos;
 *   e.mode = EmitterMode::CONTINUOUS;
 *   e.rate = 60.0f;
 *   e.velocity_min/max, e.spread_angle, e.color_start/end, etc.
 *   ps.emit(emitter_index, 10);   // burst 10 particles
 *   ps.update(delta);
 *   ps.draw(renderer);
 */
#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/color.hpp"
#include "nova/renderer/renderer2d.hpp"
#include <vector>

namespace nova {

enum class EmitterMode : u8 {
    CONTINUOUS,
    BURST,
    TRAIL
};

struct ParticleEmitter {
    Vector2f position{0, 0};
    f32 rotation = 0.0f;           // base emission direction (radians)
    f32 spread_angle = 0.5f;       // half-angle spread (radians)

    f32 velocity_min = 50.0f;
    f32 velocity_max = 150.0f;

    f32 lifetime_min = 0.5f;
    f32 lifetime_max = 1.5f;

    f32 size_start = 8.0f;
    f32 size_end = 2.0f;

    Color color_start = Color::WHITE();
    Color color_end = Color(1, 1, 1, 0);

    f32 gravity_scale = 0.0f;
    f32 drag = 1.0f;               // 0 = no drag, higher = more drag

    EmitterMode mode = EmitterMode::CONTINUOUS;
    f32 rate = 30.0f;              // particles per second (CONTINUOUS mode)
    i32 burst_count = 20;          // particles per burst (BURST mode)

    bool active = true;

    // Internal accumulator for continuous emission
    f32 _accumulator = 0.0f;
};

class ParticleSystem {
public:
    explicit ParticleSystem(usize max_particles = 2048);

    /// Add an emitter configuration, returns its index.
    usize add_emitter(const ParticleEmitter& emitter = {});

    /// Get emitter by index for modification.
    ParticleEmitter& get_emitter(usize index) { return m_emitters[index]; }

    /// Burst-emit a number of particles from an emitter.
    void emit(usize emitter_index, i32 count);

    /// Update all particles.
    void update(f32 delta);

    /// Draw all alive particles.
    void draw(Renderer2D& renderer);

    usize alive_count() const { return m_alive_count; }

private:
    struct Particle {
        Vector2f position;
        Vector2f velocity;
        f32 lifetime;
        f32 max_lifetime;
        f32 size_start;
        f32 size_end;
        Color color_start;
        Color color_end;
        f32 gravity_scale;
        f32 drag;
        bool alive = false;
    };

    void spawn_particle(const ParticleEmitter& emitter);

    std::vector<Particle> m_pool;
    std::vector<ParticleEmitter> m_emitters;
    usize m_alive_count = 0;
    usize m_next_free = 0;  // hint for pool scanning
};

} // namespace nova

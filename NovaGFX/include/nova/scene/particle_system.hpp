/**
 * ParticleSystem — GPU-instanced, highly optimized particle system.
 *
 * Design:
 *   - SoA (Structure of Arrays) with u8 alive flags (NOT vector<bool>)
 *   - Free-list stack for O(1) particle allocation, no scanning
 *   - High-watermark tracking: physics only loops up to the last used slot
 *   - Persistent mapped GPU buffer (GL_MAP_PERSISTENT_BIT + GL_MAP_COHERENT_BIT)
 *     for zero-copy, zero-stall uploads on GL 4.4+, fallback to orphaning
 *   - Single glDrawArraysInstanced call per frame (1 draw call total)
 *   - Instance data packed tightly: only 28 bytes per particle on GPU
 *
 * API (unchanged from v1):
 *   ParticleSystem ps(65536);
 *   usize id = ps.add_emitter(config);
 *   ps.emit(id, count);
 *   ps.update(delta);
 *   ps.draw(renderer);
 */
#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/color.hpp"
#include "nova/renderer/renderer2d.hpp"
#include "nova/assets/shader.hpp"
#include <vector>
#include <memory>
#include <cstdint>

namespace nova {

enum class EmitterMode : u8 {
    CONTINUOUS,
    BURST,
    TRAIL
};

struct ParticleEmitter {
    Vector2f position{0, 0};
    f32 rotation = 0.0f;
    f32 spread_angle = 0.5f;

    f32 velocity_min = 50.0f;
    f32 velocity_max = 150.0f;

    f32 lifetime_min = 0.5f;
    f32 lifetime_max = 1.5f;

    f32 size_start = 8.0f;
    f32 size_end   = 2.0f;

    Color color_start = Color::WHITE();
    Color color_end   = Color(1, 1, 1, 0);

    f32 gravity_scale = 0.0f;
    f32 drag = 1.0f;

    EmitterMode mode = EmitterMode::CONTINUOUS;
    f32 rate         = 30.0f;
    i32 burst_count  = 20;

    bool active = true;

    f32 _accumulator = 0.0f;
};

// Per-instance data uploaded to GPU each frame — 28 bytes, tightly packed
#pragma pack(push, 1)
struct ParticleInstance {
    f32 x, y;
    f32 size;
    f32 r, g, b, a;
};
#pragma pack(pop)

class ParticleSystem {
public:
    explicit ParticleSystem(usize max_particles = 65536);
    ~ParticleSystem();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    usize add_emitter(const ParticleEmitter& emitter = {});
    ParticleEmitter& get_emitter(usize index) { return m_emitters[index]; }

    void emit(usize emitter_index, i32 count);
    void update(f32 delta);
    void draw(Renderer2D& renderer);

    usize alive_count() const { return m_alive_count; }

private:
    void spawn_particle(const ParticleEmitter& emitter);
    void init_gpu();

    // ── SoA particle pool ────────────────────────────────────────────────
    usize m_capacity   = 0;
    usize m_alive_count = 0;
    usize m_high_water  = 0;   // highest slot ever used — limits loop range

    // Alive flag as u8 (NOT bool) — avoids vector<bool> bit-packing
    std::vector<uint8_t>  m_alive;

    // Free-list stack for O(1) allocation
    std::vector<uint32_t> m_free_list;

    // Physics SoA arrays (f32 for SIMD-friendly auto-vectorization)
    std::vector<f32>      m_px, m_py;
    std::vector<f32>      m_vx, m_vy;
    std::vector<f32>      m_life, m_max_life;
    std::vector<f32>      m_size_start, m_size_end;
    std::vector<f32>      m_gravity, m_drag;
    std::vector<f32>      m_cs_r, m_cs_g, m_cs_b, m_cs_a;
    std::vector<f32>      m_ce_r, m_ce_g, m_ce_b, m_ce_a;

    std::vector<ParticleEmitter> m_emitters;

    // ── GPU objects ──────────────────────────────────────────────────────
    u32  m_quad_vao = 0;
    u32  m_quad_vbo = 0;
    u32  m_inst_vbo = 0;
    std::unique_ptr<Shader> m_shader;

    // CPU staging buffer — pre-allocated, no dynamic allocations per frame
    std::vector<ParticleInstance> m_instance_buf;
};

} // namespace nova

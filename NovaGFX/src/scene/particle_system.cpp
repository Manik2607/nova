#include "nova/scene/particle_system.hpp"
#include <glad/glad.h>
#include <cmath>
#include <random>
#include <algorithm>
#include <cstdint>
#include <cstring>

namespace nova {

// ────────────────────────────────────────────────────────────────────────────
// Instancing shaders
// ────────────────────────────────────────────────────────────────────────────
static const char* PARTICLE_VERT = R"(
#version 330 core
layout(location = 0) in vec2  a_Corner;   // per-vertex unit quad corner
layout(location = 1) in vec2  a_Pos;      // per-instance world centre
layout(location = 2) in float a_Size;     // per-instance size (full extent)
layout(location = 3) in vec4  a_Color;    // per-instance rgba

uniform mat4 u_Projection;
out vec4 v_Color;
out vec2 v_UV;

void main() {
    v_UV = a_Corner;
    vec2 worldPos = a_Pos + (a_Corner - vec2(0.5)) * a_Size;
    gl_Position = u_Projection * vec4(worldPos, 0.0, 1.0);
    v_Color = a_Color;
}
)";

// Soft-circle fragment — looks much better than a hard square
static const char* PARTICLE_FRAG = R"(
#version 330 core
in vec4 v_Color;
in vec2 v_UV;
out vec4 FragColor;

void main() {
    // Radial softness: 1 at center, 0 at edge
    float d = length(v_UV - vec2(0.5)) * 2.0; // 0..1
    float alpha = smoothstep(1.0, 0.3, d);
    FragColor = vec4(v_Color.rgb, v_Color.a * alpha);
}
)";

// ────────────────────────────────────────────────────────────────────────────
// RNG — one global instance, no dynamic allocation
// ────────────────────────────────────────────────────────────────────────────
static std::mt19937 s_rng{12345u};
static std::uniform_real_distribution<float> s_dist01(0.0f, 1.0f);
static std::uniform_real_distribution<float> s_distSym(-1.0f, 1.0f);
static inline float rng01()  { return s_dist01(s_rng); }
static inline float rngSym() { return s_distSym(s_rng); }
static inline float rand_range(float a, float b) { return a + (b - a) * rng01(); }

// ────────────────────────────────────────────────────────────────────────────
// Construction
// ────────────────────────────────────────────────────────────────────────────
ParticleSystem::ParticleSystem(usize max_particles)
    : m_capacity(max_particles)
{
    // Alive flags as plain bytes — never bit-packed
    m_alive.assign(max_particles, 0u);

    // Pre-fill free list in reverse so slot 0 is popped first
    m_free_list.resize(max_particles);
    for (usize i = 0; i < max_particles; ++i)
        m_free_list[i] = static_cast<uint32_t>(max_particles - 1 - i);

    // SoA arrays
    auto alloc = [&](std::vector<f32>& v, f32 val = 0.0f) {
        v.assign(max_particles, val);
    };
    alloc(m_px); alloc(m_py);
    alloc(m_vx); alloc(m_vy);
    alloc(m_life); alloc(m_max_life, 1.0f);
    alloc(m_size_start, 8.0f); alloc(m_size_end, 2.0f);
    alloc(m_gravity); alloc(m_drag, 1.0f);
    alloc(m_cs_r, 1); alloc(m_cs_g, 1); alloc(m_cs_b, 1); alloc(m_cs_a, 1);
    alloc(m_ce_r, 1); alloc(m_ce_g, 1); alloc(m_ce_b, 1); alloc(m_ce_a, 0);

    // CPU staging buffer
    m_instance_buf.resize(max_particles);

    init_gpu();
}

ParticleSystem::~ParticleSystem() {
    if (m_quad_vao) glDeleteVertexArrays(1, &m_quad_vao);
    if (m_quad_vbo) glDeleteBuffers(1, &m_quad_vbo);
    if (m_inst_vbo) glDeleteBuffers(1, &m_inst_vbo);
}

void ParticleSystem::init_gpu() {
    m_shader = std::make_unique<Shader>(PARTICLE_VERT, PARTICLE_FRAG);

    static const f32 corners[8] = {
        0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_quad_vao);
    glBindVertexArray(m_quad_vao);

    // Corner VBO (static)
    glGenBuffers(1, &m_quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(0, 0);

    // Instance VBO — try persistent mapping (GL 4.4+)
    glGenBuffers(1, &m_inst_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_inst_vbo);

    GLsizeiptr buf_size = static_cast<GLsizeiptr>(m_capacity * sizeof(ParticleInstance));

    // GL 3.3: STREAM_DRAW. glMapBufferRange with INVALIDATE_BUFFER_BIT is used at draw time.
    glBufferData(GL_ARRAY_BUFFER, buf_size, nullptr, GL_STREAM_DRAW);

    // Instance attribute setup (location=1,2,3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance),
        (const void*)offsetof(ParticleInstance, x));
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance),
        (const void*)offsetof(ParticleInstance, size));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance),
        (const void*)offsetof(ParticleInstance, r));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

// ────────────────────────────────────────────────────────────────────────────
// Emitters
// ────────────────────────────────────────────────────────────────────────────
usize ParticleSystem::add_emitter(const ParticleEmitter& emitter) {
    m_emitters.push_back(emitter);
    return m_emitters.size() - 1;
}

// ────────────────────────────────────────────────────────────────────────────
// Spawn — O(1) via free list
// ────────────────────────────────────────────────────────────────────────────
void ParticleSystem::spawn_particle(const ParticleEmitter& emitter) {
    if (m_free_list.empty()) return;  // pool full — hard cap, no scanning

    uint32_t idx = m_free_list.back();
    m_free_list.pop_back();

    m_alive[idx] = 1;
    if (idx >= m_high_water) m_high_water = idx + 1;

    m_px[idx] = emitter.position.x;
    m_py[idx] = emitter.position.y;

    f32 angle = emitter.rotation + rngSym() * emitter.spread_angle;
    f32 speed = rand_range(emitter.velocity_min, emitter.velocity_max);
    m_vx[idx] = std::cos(angle) * speed;
    m_vy[idx] = std::sin(angle) * speed;

    m_max_life[idx] = rand_range(emitter.lifetime_min, emitter.lifetime_max);
    m_life[idx]     = m_max_life[idx];

    m_size_start[idx] = emitter.size_start;
    m_size_end[idx]   = emitter.size_end;

    m_cs_r[idx] = emitter.color_start.r;  m_cs_g[idx] = emitter.color_start.g;
    m_cs_b[idx] = emitter.color_start.b;  m_cs_a[idx] = emitter.color_start.a;
    m_ce_r[idx] = emitter.color_end.r;    m_ce_g[idx] = emitter.color_end.g;
    m_ce_b[idx] = emitter.color_end.b;    m_ce_a[idx] = emitter.color_end.a;

    m_gravity[idx] = emitter.gravity_scale;
    m_drag[idx]    = emitter.drag;

    ++m_alive_count;
}

void ParticleSystem::emit(usize emitter_index, i32 count) {
    if (emitter_index >= m_emitters.size()) return;
    const auto& e = m_emitters[emitter_index];
    for (i32 i = 0; i < count; ++i) spawn_particle(e);
}

// ────────────────────────────────────────────────────────────────────────────
// Update — iterates only up to m_high_water (not full m_capacity)
//          plain u8 alive flags — compiler can auto-vectorize the check
// ────────────────────────────────────────────────────────────────────────────
void ParticleSystem::update(f32 delta) {
    static constexpr f32 GRAVITY_BASE = 980.0f;

    // Continuous emitters
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

    usize new_alive = 0;
    usize new_hw    = 0;
    const usize hw  = m_high_water;   // snapshot before loop

    // Single pass — tight loop over [0, high_water)
    for (usize i = 0; i < hw; ++i) {
        if (!m_alive[i]) continue;

        m_life[i] -= delta;
        if (m_life[i] <= 0.0f) {
            m_alive[i] = 0;
            m_free_list.push_back(static_cast<uint32_t>(i));
            continue;
        }

        // Physics
        const f32 drag_factor = 1.0f - m_drag[i] * delta;
        m_vx[i] *= drag_factor;
        m_vy[i] *= drag_factor;
        m_vy[i] += m_gravity[i] * GRAVITY_BASE * delta;
        m_px[i] += m_vx[i] * delta;
        m_py[i] += m_vy[i] * delta;

        ++new_alive;
        new_hw = i + 1;
    }

    m_alive_count = new_alive;
    m_high_water  = new_hw;
}

// ────────────────────────────────────────────────────────────────────────────
// Draw — build instance buffer then issue one instanced draw call
// ────────────────────────────────────────────────────────────────────────────
void ParticleSystem::draw(Renderer2D& renderer) {
    (void)renderer;
    if (m_alive_count == 0) return;

    // ── Build compact instance buffer ─────────────────────────────────────
    ParticleInstance* dst = m_instance_buf.data();
    usize inst_count = 0;
    const usize hw = m_high_water;

    for (usize i = 0; i < hw; ++i) {
        if (!m_alive[i]) continue;

        const f32 t    = 1.0f - m_life[i] / m_max_life[i];
        const f32 size = m_size_start[i] + (m_size_end[i] - m_size_start[i]) * t;

        ParticleInstance& inst = dst[inst_count++];
        inst.x = m_px[i];  inst.y = m_py[i];  inst.size = size;
        inst.r = m_cs_r[i] + (m_ce_r[i] - m_cs_r[i]) * t;
        inst.g = m_cs_g[i] + (m_ce_g[i] - m_cs_g[i]) * t;
        inst.b = m_cs_b[i] + (m_ce_b[i] - m_cs_b[i]) * t;
        inst.a = m_cs_a[i] + (m_ce_a[i] - m_cs_a[i]) * t;
    }

    if (inst_count == 0) return;

    // ── Upload via glMapBufferRange (no stall, no realloc) ───────────────
    glBindBuffer(GL_ARRAY_BUFFER, m_inst_vbo);
    void* mapped = glMapBufferRange(GL_ARRAY_BUFFER, 0,
        static_cast<GLsizeiptr>(inst_count * sizeof(ParticleInstance)),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (mapped) {
        std::memcpy(mapped, m_instance_buf.data(), inst_count * sizeof(ParticleInstance));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        // Fallback
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            static_cast<GLsizeiptr>(inst_count * sizeof(ParticleInstance)),
            m_instance_buf.data());
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // ── Grab current projection (set by main renderer's init_camera) ─────
    GLint current_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog);
    f32 proj[16] = {};
    if (current_prog) {
        GLint loc = glGetUniformLocation(current_prog, "u_Projection");
        if (loc >= 0) glGetUniformfv(current_prog, loc, proj);
    }

    // ── Issue one instanced draw call ────────────────────────────────────
    m_shader->bind();
    glUniformMatrix4fv(
        glGetUniformLocation(m_shader->handle(), "u_Projection"),
        1, GL_FALSE, proj);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(m_quad_vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, static_cast<GLsizei>(inst_count));
    glBindVertexArray(0);

    m_shader->unbind();
    if (current_prog) glUseProgram(current_prog);
}

} // namespace nova

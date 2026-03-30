#include "neon_renderer.hpp"
#include <cmath>
#include <algorithm>

void NeonRenderer::init(i32 viewport_w, i32 viewport_h) {
    m_post_process.init(viewport_w, viewport_h);

    // Configure post-processing for neon look
    m_post_process.set_bloom_enabled(true);
    m_post_process.set_bloom_threshold(0.6f);
    m_post_process.set_bloom_intensity(1.5f);
    m_post_process.set_vignette_enabled(true);
    m_post_process.set_vignette_intensity(0.5f);
    m_post_process.set_chromatic_aberration_enabled(true);
    m_post_process.set_chromatic_aberration_strength(0.002f);

    // ── Tire smoke emitter ──
    ParticleEmitter smoke;
    smoke.velocity_min = 10.0f;
    smoke.velocity_max = 40.0f;
    smoke.spread_angle = 3.14159f; // full circle
    smoke.lifetime_min = 0.3f;
    smoke.lifetime_max = 0.8f;
    smoke.size_start = 12.0f;
    smoke.size_end = 24.0f;
    smoke.color_start = Color(0.5f, 0.5f, 0.6f, 0.5f);
    smoke.color_end = Color(0.3f, 0.3f, 0.4f, 0.0f);
    smoke.drag = 3.0f;
    smoke.mode = EmitterMode::CONTINUOUS;
    smoke.rate = 0.0f; // controlled manually
    smoke.active = false;
    m_tire_smoke_emitter = m_particles.add_emitter(smoke);

    // ── Sparks emitter ──
    ParticleEmitter sparks;
    sparks.velocity_min = 200.0f;
    sparks.velocity_max = 500.0f;
    sparks.spread_angle = 0.8f;
    sparks.lifetime_min = 0.1f;
    sparks.lifetime_max = 0.4f;
    sparks.size_start = 3.0f;
    sparks.size_end = 1.0f;
    sparks.color_start = Color(1.0f, 0.9f, 0.3f, 1.0f);
    sparks.color_end = Color(1.0f, 0.3f, 0.0f, 0.0f);
    sparks.drag = 1.5f;
    sparks.gravity_scale = 0.0f;
    sparks.mode = EmitterMode::BURST;
    sparks.active = false;
    m_sparks_emitter = m_particles.add_emitter(sparks);

    // ── Speed lines emitter ──
    ParticleEmitter speed_lines;
    speed_lines.velocity_min = 100.0f;
    speed_lines.velocity_max = 300.0f;
    speed_lines.spread_angle = 0.2f;
    speed_lines.lifetime_min = 0.1f;
    speed_lines.lifetime_max = 0.25f;
    speed_lines.size_start = 2.0f;
    speed_lines.size_end = 1.0f;
    speed_lines.color_start = Color(0.3f, 0.8f, 1.0f, 0.6f);
    speed_lines.color_end = Color(0.3f, 0.8f, 1.0f, 0.0f);
    speed_lines.drag = 0.5f;
    speed_lines.mode = EmitterMode::CONTINUOUS;
    speed_lines.rate = 0.0f;
    speed_lines.active = false;
    m_speed_lines_emitter = m_particles.add_emitter(speed_lines);
}

void NeonRenderer::resize(i32 w, i32 h) {
    m_post_process.resize(w, h);
}

void NeonRenderer::update(f32 dt, const Car& car) {
    Vector2f pos = car.get_position();
    f32 speed = car.get_speed();
    bool drifting = car.is_drifting();

    // ── Tire smoke ──
    auto& smoke = m_particles.get_emitter(m_tire_smoke_emitter);
    smoke.position = pos;
    smoke.active = drifting;
    smoke.rate = drifting ? (30.0f + car.get_slip_angle() * 40.0f) : 0.0f;

    // ── Tire marks (decals) ──
    if (drifting || speed > 300.0f) {
        Vector2f rear_left, rear_right;
        car.get_rear_wheel_positions(rear_left, rear_right);

        if (m_has_prev) {
            f32 mark_width = drifting ? 4.0f : 2.0f;
            Color mark_color = drifting
                ? Color(0.15f, 0.12f, 0.1f, 0.8f) // dark rubber
                : Color(0.1f, 0.1f, 0.1f, 0.4f);   // subtle

            m_decals.add_mark(m_prev_rear_left, rear_left, mark_width, mark_color, 8.0f);
            m_decals.add_mark(m_prev_rear_right, rear_right, mark_width, mark_color, 8.0f);
        }

        m_prev_rear_left = rear_left;
        m_prev_rear_right = rear_right;
        m_has_prev = true;
    } else {
        m_has_prev = false;
    }

    // ── Speed lines at high speed ──
    auto& speed_line = m_particles.get_emitter(m_speed_lines_emitter);
    speed_line.position = pos;
    f32 speed_ratio = speed / 800.0f;
    if (speed_ratio > 0.5f) {
        speed_line.active = true;
        speed_line.rate = speed_ratio * 40.0f;
        // Emit behind the car
        speed_line.rotation = std::atan2(-car.get_forward().y, -car.get_forward().x);
    } else {
        speed_line.active = false;
    }

    // ── Post-processing reactivity ──
    // Increase bloom and chroma during drifts and high speed
    f32 target_bloom = 1.2f + (drifting ? 0.5f : 0.0f) + speed_ratio * 0.3f;
    f32 target_chroma = 0.002f + (drifting ? 0.004f : 0.0f) + speed_ratio * 0.002f;
    m_post_process.set_bloom_intensity(target_bloom);
    m_post_process.set_chromatic_aberration_strength(target_chroma);

    // Update systems
    m_particles.update(dt);
    m_decals.update(dt);
}

void NeonRenderer::draw_behind(Renderer2D& renderer) {
    m_decals.draw(renderer);
}

void NeonRenderer::draw_front(Renderer2D& renderer) {
    m_particles.draw(renderer);
}

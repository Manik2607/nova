#pragma once
#include <nova/nova.hpp>
#include "nova/scene/particle_system.hpp"
#include "nova/scene/decal_system.hpp"
#include "nova/renderer/post_process.hpp"
#include "nova/scene/camera2d.hpp"
#include "car.hpp"

using namespace nova;

/// Coordinates all neon-synthwave visual effects.
class NeonRenderer {
public:
    void init(i32 viewport_w, i32 viewport_h);
    void resize(i32 w, i32 h);

    /// Update effects based on car state.
    void update(f32 dt, const Car& car);

    /// Draw particles behind the car.
    void draw_behind(Renderer2D& renderer);
    /// Draw particles in front of the car.
    void draw_front(Renderer2D& renderer);

    PostProcessPipeline& get_post_process() { return m_post_process; }
    DecalSystem& get_decals() { return m_decals; }

private:
    ParticleSystem m_particles{4096};
    DecalSystem m_decals{1024};
    PostProcessPipeline m_post_process;

    usize m_tire_smoke_emitter = 0;
    usize m_sparks_emitter = 0;
    usize m_speed_lines_emitter = 0;

    // Trail tracking
    Vector2f m_prev_rear_left{0, 0};
    Vector2f m_prev_rear_right{0, 0};
    bool m_has_prev = false;
};

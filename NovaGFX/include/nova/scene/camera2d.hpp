/**
 * Camera2D — Engine-level 2D camera with smooth follow, screen shake, and zoom.
 *
 * API Surface:
 *   Camera2D cam(viewport_width, viewport_height);
 *   cam.follow(target_pos, delta);        // smooth lerp follow + velocity lookahead
 *   cam.add_trauma(0.5f);                 // screen shake (trauma² intensity, auto-decays)
 *   cam.set_target_zoom(2.0f);            // smooth zoom transition
 *   cam.update(delta);                    // must call each frame
 *   Mat4 view = cam.get_projection();     // use for renderer
 *   Rect2f bounds = cam.get_view_bounds();// for frustum culling
 */
#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/mat4.hpp"
#include "nova/math/rect2.hpp"
#include <random>

namespace nova {

struct CameraConfig {
    f32 follow_lerp    = 5.0f;    // how quickly camera catches up (higher = faster)
    f32 lookahead_dist = 200.0f;  // pixels ahead in velocity direction
    f32 lookahead_lerp = 3.0f;    // how quickly lookahead adjusts
    f32 zoom_lerp      = 4.0f;    // zoom interpolation speed
    f32 shake_decay    = 2.0f;    // trauma decay rate per second
    f32 max_shake_offset = 20.0f; // max pixel offset from shake
    f32 max_shake_rotation = 0.05f; // max rotation from shake (radians)
};

class Camera2D {
public:
    Camera2D(f32 viewport_w, f32 viewport_h);

    /// Smoothly follow a target position with velocity-based lookahead.
    void follow(Vector2f target_pos, Vector2f target_velocity, f32 delta);

    /// Set position directly (no interpolation).
    void set_position(Vector2f pos) { m_position = pos; m_actual_position = pos; }

    /// Add trauma for screen shake (0-1 range, clamped).
    void add_trauma(f32 amount);

    /// Set the target zoom level (smoothly interpolated).
    void set_target_zoom(f32 zoom) { m_target_zoom = zoom; }

    /// Set zoom immediately.
    void set_zoom(f32 zoom) { m_zoom = zoom; m_target_zoom = zoom; }

    /// Set world boundaries the camera won't exceed (0,0,0,0 = no bounds).
    void set_bounds(Rect2f bounds) { m_bounds = bounds; m_use_bounds = true; }
    void clear_bounds() { m_use_bounds = false; }

    /// Update camera state (call once per frame).
    void update(f32 delta);

    /// Resize viewport.
    void set_viewport(f32 w, f32 h) { m_viewport = {w, h}; }

    /// Get the final projection matrix (includes position, zoom, shake).
    Mat4 get_projection() const;

    /// Get the axis-aligned view bounds in world space (for culling).
    Rect2f get_view_bounds() const;

    /// Getters
    Vector2f get_position() const { return m_actual_position; }
    f32 get_zoom() const { return m_zoom; }
    f32 get_trauma() const { return m_trauma; }
    Vector2f get_viewport() const { return m_viewport; }

    CameraConfig config;

private:
    Vector2f m_position{0, 0};       // target-tracked position
    Vector2f m_actual_position{0, 0}; // final position after shake
    Vector2f m_lookahead{0, 0};      // current lookahead offset
    Vector2f m_viewport;

    f32 m_zoom = 1.0f;
    f32 m_target_zoom = 1.0f;
    f32 m_trauma = 0.0f;             // 0-1, shake = trauma²

    Vector2f m_shake_offset{0, 0};

    bool m_use_bounds = false;
    Rect2f m_bounds{};

    std::mt19937 m_rng{42};
    std::uniform_real_distribution<f32> m_shake_dist{-1.0f, 1.0f};
};

} // namespace nova

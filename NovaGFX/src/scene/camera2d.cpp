#include "nova/scene/camera2d.hpp"
#include <algorithm>
#include <cmath>

namespace nova {

Camera2D::Camera2D(f32 viewport_w, f32 viewport_h)
    : m_viewport(viewport_w, viewport_h)
{}

void Camera2D::follow(Vector2f target_pos, Vector2f target_velocity, f32 delta) {
    // Compute lookahead from velocity direction
    Vector2f desired_lookahead = {0, 0};
    f32 speed = target_velocity.length();
    if (speed > 1.0f) {
        desired_lookahead = target_velocity.normalized() * config.lookahead_dist;
    }
    m_lookahead = m_lookahead.lerp(desired_lookahead, config.lookahead_lerp * delta);

    // Smooth follow to target + lookahead
    Vector2f target = target_pos + m_lookahead;
    m_position = m_position.lerp(target, config.follow_lerp * delta);
}

void Camera2D::add_trauma(f32 amount) {
    m_trauma = std::min(1.0f, m_trauma + amount);
}

void Camera2D::update(f32 delta) {
    // Decay trauma
    m_trauma = std::max(0.0f, m_trauma - config.shake_decay * delta);

    // Smooth zoom
    m_zoom = m_zoom + (m_target_zoom - m_zoom) * config.zoom_lerp * delta;
    if (m_zoom < 0.01f) m_zoom = 0.01f;

    // Compute shake offset (shake intensity = trauma²)
    f32 shake_amount = m_trauma * m_trauma;
    m_shake_offset = {
        shake_amount * config.max_shake_offset * m_shake_dist(m_rng),
        shake_amount * config.max_shake_offset * m_shake_dist(m_rng)
    };

    // Compute final position
    m_actual_position = m_position + m_shake_offset;

    // Clamp to world bounds if set
    if (m_use_bounds) {
        Vector2f half_view = m_viewport / (2.0f * m_zoom);
        f32 min_x = m_bounds.position.x + half_view.x;
        f32 max_x = m_bounds.position.x + m_bounds.size.x - half_view.x;
        f32 min_y = m_bounds.position.y + half_view.y;
        f32 max_y = m_bounds.position.y + m_bounds.size.y - half_view.y;

        if (min_x < max_x) {
            m_actual_position.x = std::clamp(m_actual_position.x, min_x, max_x);
        }
        if (min_y < max_y) {
            m_actual_position.y = std::clamp(m_actual_position.y, min_y, max_y);
        }
    }
}

Mat4 Camera2D::get_projection() const {
    Vector2f size = m_viewport / m_zoom;
    Vector2f left_top = m_actual_position - size * 0.5f;
    Vector2f right_bottom = m_actual_position + size * 0.5f;
    return Mat4::ortho(left_top.x, right_bottom.x, right_bottom.y, left_top.y, -1.0f, 1.0f);
}

Rect2f Camera2D::get_view_bounds() const {
    Vector2f size = m_viewport / m_zoom;
    Vector2f left_top = m_actual_position - size * 0.5f;
    return {left_top, size};
}

} // namespace nova

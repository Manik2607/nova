#pragma once
#include <nova/nova.hpp>
using namespace nova;

/// Combo-based drift scoring system.
/// drift_angle × speed × duration → multiplier chain.
class DriftScorer {
public:
    void update(f32 dt, bool is_drifting, f32 slip_angle, f32 speed);
    void draw(Renderer2D& renderer, Vector2f screen_pos);

    i32 get_total_score() const { return m_total_score; }
    i32 get_current_combo() const { return m_current_combo; }
    f32 get_current_drift_score() const { return m_drift_score; }
    bool is_in_drift() const { return m_in_drift; }

private:
    bool m_in_drift = false;
    f32 m_drift_timer = 0.0f;
    f32 m_drift_score = 0.0f;
    i32 m_current_combo = 0;
    i32 m_total_score = 0;

    // Grace period: brief non-drift moment doesn't break combo
    f32 m_grace_timer = 0.0f;
    static constexpr f32 GRACE_PERIOD = 0.3f;
};

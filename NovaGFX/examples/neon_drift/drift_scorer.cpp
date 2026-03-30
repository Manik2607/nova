#include "drift_scorer.hpp"
#include <cmath>
#include <algorithm>

void DriftScorer::update(f32 dt, bool is_drifting, f32 slip_angle, f32 speed) {
    if (is_drifting) {
        if (!m_in_drift) {
            m_in_drift = true;
            m_drift_timer = 0.0f;
            m_drift_score = 0.0f;
            m_current_combo++;
        }

        m_grace_timer = GRACE_PERIOD;
        m_drift_timer += dt;

        // Score accumulation: slip_angle × speed × dt
        f32 score_rate = slip_angle * speed * 0.01f;
        // Duration bonus: longer drifts score more per second
        f32 duration_mult = 1.0f + m_drift_timer * 0.5f;
        m_drift_score += score_rate * duration_mult * dt;
    } else {
        m_grace_timer -= dt;
        if (m_grace_timer <= 0.0f && m_in_drift) {
            // Drift ended — bank the score
            i32 base_points = static_cast<i32>(m_drift_score * 10.0f);
            i32 combo_mult = std::min(m_current_combo, 10);
            m_total_score += base_points * combo_mult;

            m_in_drift = false;
            m_drift_score = 0.0f;
            m_drift_timer = 0.0f;
            m_current_combo = 0;
        }
    }
}

void DriftScorer::draw(Renderer2D& /*renderer*/, Vector2f /*screen_pos*/) {
    // HUD draws the scoring — this is a stub for direct rendering if needed
}

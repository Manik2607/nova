#pragma once
#include <nova/nova.hpp>
#include <box2d/box2d.h>
#include "nova/scene/camera2d.hpp"
#include "nova/renderer/text_renderer.hpp"
#include "car.hpp"
#include "track.hpp"
#include "drift_scorer.hpp"
#include "hud.hpp"
#include "neon_renderer.hpp"
#include <memory>

using namespace nova;

enum class GameState {
    PLAYING,
    PAUSED
};

/// Top-level game class owning all systems.
class Game {
public:
    Game(i32 viewport_w, i32 viewport_h);

    void update(f32 dt);
    void draw(Renderer2D& renderer);
    void resize(i32 w, i32 h);

    GameState get_state() const { return m_state; }

private:
    GameState m_state = GameState::PLAYING;

    // Physics
    std::unique_ptr<b2World> m_world;

    // Systems
    Camera2D m_camera;
    Track m_track;
    std::unique_ptr<Car> m_car;
    DriftScorer m_scorer;
    HUD m_hud;
    NeonRenderer m_neon;
    TextRenderer m_text;

    i32 m_viewport_w, m_viewport_h;

    // Timing
    f32 m_fps = 0.0f;
    f32 m_fps_timer = 0.0f;
    i32 m_frame_count = 0;
};

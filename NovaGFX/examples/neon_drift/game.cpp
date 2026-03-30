#include "game.hpp"
#include <iostream>

Game::Game(i32 viewport_w, i32 viewport_h)
    : m_camera(static_cast<f32>(viewport_w), static_cast<f32>(viewport_h)),
      m_viewport_w(viewport_w), m_viewport_h(viewport_h)
{
    // Zero-gravity world for top-down driving
    m_world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f));

    // Generate the track
    m_track.generate(m_world.get(), 42);

    // Create the car at the track's start position
    m_car = std::make_unique<Car>(m_world.get(),
                                  m_track.get_start_position(),
                                  m_track.get_start_angle());

    // Camera config for racing feel
    m_camera.config.follow_lerp = 6.0f;
    m_camera.config.lookahead_dist = 250.0f;
    m_camera.config.lookahead_lerp = 3.5f;
    m_camera.config.shake_decay = 3.0f;
    m_camera.set_zoom(0.01f);
    m_camera.set_position(m_track.get_start_position());

    // Initialize neon effects
    m_neon.init(viewport_w, viewport_h);
}

void Game::update(f32 dt) {
    if (m_state == GameState::PAUSED) return;

    // Toggle pause
    if (Input::is_key_pressed(Key::P)) {
        m_state = GameState::PAUSED;
        return;
    }

    // Get surface under car
    SurfaceType surface = m_track.get_tilemap().get_surface_at(m_car->get_position());
    f32 grip = get_grip_for_surface(surface);

    // Sub-step physics to bypass Box2D's hard speed limit (b2_maxTranslation).
    // Box2D limits objects to moving 2.0 units per step. By stepping multiple times
    // per frame, we multiply the absolute top speed limit!
    int sub_steps = 5;
    f32 sub_dt = dt / sub_steps;
    for (int i = 0; i < sub_steps; ++i) {
        // Update car forces each sub-step so Box2D doesn't clear them
        m_car->update(sub_dt, grip);
        m_world->Step(sub_dt, 8, 3);
    }

    // Update camera
    m_camera.follow(m_car->get_position(), m_car->get_velocity(), dt);

    // Camera shake from wall hits (detect high deceleration)
    f32 speed = m_car->get_speed();
    if (speed > 200.0f && grip < 0.5f) {
        m_camera.add_trauma(0.02f);
    }

    // Zoom based on speed
    f32 target_zoom = 0.7f - (speed / 800.0f) * 0.15f; // zoom out at speed
    m_camera.set_target_zoom(target_zoom);

    m_camera.update(dt);

    // Update drift scoring
    m_scorer.update(dt, m_car->is_drifting(), m_car->get_slip_angle(), speed);

    // Update visual effects
    m_neon.update(dt, *m_car);

    // FPS tracking
    m_frame_count++;
    m_fps_timer += dt;
    if (m_fps_timer >= 1.0f) {
        m_fps = static_cast<f32>(m_frame_count);
        m_frame_count = 0;
        m_fps_timer -= 1.0f;
        std::cout << "FPS: " << static_cast<i32>(m_fps)
                  << " | Score: " << m_scorer.get_total_score()
                  << " | Speed: " << static_cast<i32>(speed)
                  << " | Drift: " << (m_car->is_drifting() ? "YES" : "no")
                  << "\n";
    }
}

void Game::draw(Renderer2D& renderer) {
    auto& pp = m_neon.get_post_process();

    // Begin post-processing scene capture
    pp.begin_scene();
    glClearColor(0.01f, 0.01f, 0.03f, 1.0f); // very dark blue-black
    glClear(GL_COLOR_BUFFER_BIT);

    // ── World-space rendering ──
    renderer.init_camera(m_camera.get_position(), m_camera.get_zoom(),
                         m_camera.get_viewport());

    renderer.begin();
    Rect2f view_bounds = m_camera.get_view_bounds();

    // Layer 1: Track tiles
    m_track.draw(renderer, view_bounds);

    // Layer 2: Decals (tire marks) — behind car
    m_neon.draw_behind(renderer);

    // Layer 3: Car
    m_car->draw(renderer);

    // Layer 4: Particles — in front of car
    m_neon.draw_front(renderer);

    renderer.end();

    // End scene capture
    pp.end_scene();

    // Run post-processing and output to screen
    pp.render();

    // ── Screen-space HUD ──
    // Set up screen-space projection
    Vector2f vp = m_camera.get_viewport();
    renderer.init_camera(vp * 0.5f, 1.0f, vp);
    renderer.begin();
    m_hud.draw(renderer, m_text, vp, m_car->get_speed(), m_scorer, m_fps);
    renderer.end();
}

void Game::resize(i32 w, i32 h) {
    m_viewport_w = w;
    m_viewport_h = h;
    m_camera.set_viewport(static_cast<f32>(w), static_cast<f32>(h));
    m_neon.resize(w, h);
}

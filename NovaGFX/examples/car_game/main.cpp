#include <nova/nova.hpp>
#include <box2d/box2d.h>
#include <iostream>
#include <chrono>
#include "car.hpp"
#include "terrain.hpp"

using namespace nova;

const float PIXELS_PER_METER = 100.0f;

int main() {
    try {
        Window window(1280, 720, "NovaGFX 2D Car Game");
        window.set_vsync(false);
        Renderer2D renderer;

        // Box2D World
        b2Vec2 gravity(0.0f, 9.8f);
        b2World world(gravity);

        // Initialize entities
        Car car(&world, Vector2f(0.0f, -500.0f)); // Start in the air
        TerrainGenerator terrain(&world);

        // Camera variables
        Vector2f camera_pos = {0, 0};
        const float CAMERA_ZOOM = 0.333f; // Zoomed out 3x

        int frame_count = 0;
        f32 fps_timer = 0.0f;
        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.should_close()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            f32 delta = std::chrono::duration<f32>(current_time - last_time).count();
            last_time = current_time;
            
            if (delta > 0.1f) delta = 0.1f;

            Input::update();
            window.poll_events();

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            // Update Logic
            car.update(delta);
            terrain.update(car.get_position().x); // Generate chunks ahead of the car

            // Step Physics
            int32 velocityIterations = 8;
            int32 positionIterations = 3;
            world.Step(delta, velocityIterations, positionIterations);

            // Update Camera Object Tracking
            Vector2f car_pos = car.get_position();
            // Smoothly track the car, placing it slightly to the left of the center (so we see ahead)
            Vector2f target_camera_pos = car_pos + Vector2f(400.0f, -100.0f);
            camera_pos = camera_pos.lerp(target_camera_pos, 5.0f * delta);

            // Rendering setup
            Vector2i size = window.get_size();
            glViewport(0, 0, size.x, size.y);
            
            // Draw Sky Gradient Fake (clear color)
            glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.init_camera(camera_pos, CAMERA_ZOOM, Vector2f(static_cast<f32>(size.x), static_cast<f32>(size.y)));

            renderer.begin();

            // Draw Background elements (e.g. distant mountains that move slower via parallax if we had a parallax system)
            // Draw Terrain
            terrain.draw(renderer);
            
            // Draw Car
            car.draw(renderer);

            // Draw loose dynamic obstacles
            for (b2Body* b = world.GetBodyList(); b; b = b->GetNext()) {
                if (b->GetType() == b2_dynamicBody && !car.is_part_of_car(b)) {
                    b2Vec2 pos = b->GetPosition();
                    float angle = b->GetAngle();
                    for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext()) {
                        if (f->GetType() == b2Shape::e_polygon) {
                            b2PolygonShape* poly = (b2PolygonShape*)f->GetShape();
                            std::vector<Vector2f> vertices;
                            for (int i = 0; i < poly->m_count; ++i) {
                                b2Vec2 v = b->GetWorldPoint(poly->m_vertices[i]);
                                vertices.push_back({v.x * PIXELS_PER_METER, v.y * PIXELS_PER_METER});
                            }
                            renderer.draw_polygon(vertices, Color(0.6f, 0.4f, 0.1f, 1.0f)); // Wood crate color
                            renderer.draw_polygon_outline(vertices, Color::BLACK(), 2.0f);
                        }
                    }
                }
            }

            // UI / HUD
            renderer.end();

            frame_count++;
            fps_timer += delta;
            if (fps_timer >= 1.0f) {
                FrameStats stats = renderer.get_stats();
                std::cout << "FPS: " << frame_count << " | Bodies: " << world.GetBodyCount() 
                          << " | Draw calls: " << stats.draw_calls << " | Verts: " << stats.vertex_count << "\n";
                frame_count = 0;
                fps_timer -= 1.0f;
            }

            window.swap_buffers();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
    }

    return 0;
}

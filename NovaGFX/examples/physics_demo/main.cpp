#include <nova/nova.hpp>
#include <box2d/box2d.h>
#include <iostream>
#include <chrono>
#include <vector>

using namespace nova;

int main() {
    try {
        Window window(1280, 720, "NovaGFX Box2D Demo");
        window.set_vsync(false);
        Renderer2D renderer;

        // Box2D World
        b2Vec2 gravity(0.0f, 9.8f); // 9.8m/s^2 downwards. Note: Box2D uses meters, so we'll scale drawing.
        b2World world(gravity);

        const float PIXELS_PER_METER = 100.0f; // 1 meter = 100 pixels

        // Static ground
        b2BodyDef groundDef;
        groundDef.position.Set(6.4f, 6.8f); // 640px / 100, 680px / 100
        b2Body* ground = world.CreateBody(&groundDef);
        b2PolygonShape groundBox;
        groundBox.SetAsBox(6.0f, 0.2f); // 600px / 100, 20px / 100
        ground->CreateFixture(&groundBox, 0.0f);

        // Platform 1
        b2BodyDef plat1Def;
        plat1Def.position.Set(3.0f, 5.0f);
        plat1Def.angle = 0.2f;
        b2Body* plat1 = world.CreateBody(&plat1Def);
        b2PolygonShape plat1Box;
        plat1Box.SetAsBox(1.5f, 0.1f);
        plat1->CreateFixture(&plat1Box, 0.0f);

        // Platform 2
        b2BodyDef plat2Def;
        plat2Def.position.Set(9.8f, 4.0f);
        plat2Def.angle = -0.2f;
        b2Body* plat2 = world.CreateBody(&plat2Def);
        b2PolygonShape plat2Box;
        plat2Box.SetAsBox(1.5f, 0.1f);
        plat2->CreateFixture(&plat2Box, 0.0f);

        u32 white_data = 0xFFFFFFFF;
        Texture2D default_tex((u8*)&white_data, 1, 1, 4);

        auto last_time = std::chrono::high_resolution_clock::now();
        int frame_count = 0;
        f32 fps_timer = 0.0f;

        while (!window.should_close()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            f32 delta = std::chrono::duration<f32>(current_time - last_time).count();
            last_time = current_time;
            
            // Limit delta to prevent instability during lag
            if (delta > 0.1f) delta = 0.1f;

            Input::update();
            window.poll_events();

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            // Spawn Box
            if (Input::is_mouse_pressed(MouseButton::LEFT)) {
                Vector2f mpos = Input::get_mouse_position();
                b2BodyDef bodyDef;
                bodyDef.type = b2_dynamicBody;
                bodyDef.position.Set(mpos.x / PIXELS_PER_METER, mpos.y / PIXELS_PER_METER);
                bodyDef.angle = static_cast<f32>(rand() % 100) / 100.0f;
                b2Body* body = world.CreateBody(&bodyDef);

                b2PolygonShape dynamicBox;
                dynamicBox.SetAsBox(0.2f, 0.2f); // 20px / 100
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &dynamicBox;
                fixtureDef.density = 1.0f;
                fixtureDef.friction = 0.3f;
                fixtureDef.restitution = 0.4f;
                body->CreateFixture(&fixtureDef);
            }
            
            // Spawn Circle
            if (Input::is_mouse_pressed(MouseButton::RIGHT)) {
                Vector2f mpos = Input::get_mouse_position();
                b2BodyDef bodyDef;
                bodyDef.type = b2_dynamicBody;
                bodyDef.position.Set(mpos.x / PIXELS_PER_METER, mpos.y / PIXELS_PER_METER);
                b2Body* body = world.CreateBody(&bodyDef);

                b2CircleShape circle;
                circle.m_radius = 0.2f; // 20px / 100
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circle;
                fixtureDef.density = 1.0f;
                fixtureDef.friction = 0.3f;
                fixtureDef.restitution = 0.8f;
                body->CreateFixture(&fixtureDef);
            }

            // Step Box2D
            int32 velocityIterations = 8;
            int32 positionIterations = 3;
            world.Step(delta, velocityIterations, positionIterations);

            Vector2i size = window.get_size();
            glViewport(0, 0, size.x, size.y);
            glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.init_camera(Vector2f(static_cast<f32>(size.x)/2.0f, static_cast<f32>(size.y)/2.0f), 1.0f, Vector2f(static_cast<f32>(size.x), static_cast<f32>(size.y)));

            renderer.begin();

            for (b2Body* b = world.GetBodyList(); b; b = b->GetNext()) {
                b2Vec2 pos = b->GetPosition();
                float angle = b->GetAngle();
                Vector2f pixel_pos = {pos.x * PIXELS_PER_METER, pos.y * PIXELS_PER_METER};
                Color color = b->GetType() == b2_staticBody ? Color::GRAY() : Color::CYAN();

                for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext()) {
                    switch (f->GetType()) {
                        case b2Shape::e_polygon: {
                            b2PolygonShape* poly = (b2PolygonShape*)f->GetShape();
                            std::vector<Vector2f> vertices;
                            for (int i = 0; i < poly->m_count; ++i) {
                                b2Vec2 v = b->GetWorldPoint(poly->m_vertices[i]);
                                vertices.push_back({v.x * PIXELS_PER_METER, v.y * PIXELS_PER_METER});
                            }
                            renderer.draw_polygon(vertices, color);
                            renderer.draw_polygon_outline(vertices, Color::BLACK(), 2.0f);
                            break;
                        }
                        case b2Shape::e_circle: {
                            b2CircleShape* circle = (b2CircleShape*)f->GetShape();
                            b2Vec2 center = b->GetWorldPoint(circle->m_p);
                            Vector2f pcenter = {center.x * PIXELS_PER_METER, center.y * PIXELS_PER_METER};
                            float radius = circle->m_radius * PIXELS_PER_METER;
                            Color c_color = b->GetType() == b2_staticBody ? Color::GRAY() : Color::ORANGE();
                            renderer.draw_circle(pcenter, radius, c_color, 32);
                            renderer.draw_circle_outline(pcenter, radius, Color::BLACK(), 2.0f, 32);
                            
                            // Rotation line
                            Vector2f end_p = pcenter + Vector2f{std::cos(angle) * radius, std::sin(angle) * radius};
                            renderer.draw_line(pcenter, end_p, Color::BLACK(), 2.0f);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

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

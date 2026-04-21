#include <nova/nova.hpp>
#include <box2d/box2d.h>
#include <iostream>
#include <chrono>
#include "car.hpp"
#include "terrain.hpp"
#include <nova/scene/camera2d.hpp>
#include <nova/scene/particle_system.hpp>
#include <nova/ui/control.hpp>
#include <nova/ui/label.hpp>
#include <nova/ui/button.hpp>
#include <nova/ui/text_input.hpp>
#include <nova/ui/box_container.hpp>
#include <nova/ui/tab_bar.hpp>
#include <nova/ui/panel.hpp>
#include <nova/ui/scroll_view.hpp>
#include <nova/renderer/text_renderer.hpp>
#include <nova/renderer/post_process.hpp>

using namespace nova;
using namespace nova::ui;

const float PIXELS_PER_METER = 100.0f;

class QueryCallback : public b2QueryCallback {
public:
    QueryCallback(const b2Vec2& point) : point(point), fixture(nullptr) {}
    bool ReportFixture(b2Fixture* f) override {
        b2Body* body = f->GetBody();
        if (body->GetType() == b2_dynamicBody) {
            if (f->TestPoint(point)) {
                fixture = f; return false;
            }
        }
        return true;
    }
    b2Vec2 point;
    b2Fixture* fixture;
};

int main() {
    try {
        Window window(1280, 720, "NovaGFX 2D Car Game");
        window.set_vsync(false);
        Renderer2D renderer;

        // Box2D World
        b2Vec2 gravity(0.0f, 9.8f);
        b2World world(gravity);

        // Initialize entities
        auto car_texture = std::make_shared<Texture2D>("examples/car_game/Images/carWithWindows.png");
        Car car(&world, Vector2f(0.0f, -500.0f), car_texture); // Start in the air
        TerrainGenerator terrain(&world);

        b2BodyDef staticDef;
        b2Body* groundBody = world.CreateBody(&staticDef);
        b2MouseJoint* mouseJoint = nullptr;

        // ── DASHBOARD UI (Modern Layout) ───────────────────────────────────
        TextRenderer text_renderer;
        bool show_ui = true;

        auto root_hbox = std::make_unique<ui::HBoxContainer>();
        root_hbox->size = {1280.0f, 720.0f};
        root_hbox->anchor = ui::Anchor::FILL;
        
        // Sidebar Panel
        auto side_panel = root_hbox->add_child<ui::Panel>();
        side_panel->size = {400, 0};
        side_panel->expand = false; // Stay at 320px
        side_panel->background_color = {0.05f, 0.05f, 0.07f, 0.95f};
        side_panel->padding = {15, 20, 15, 20};

        auto side_vbox = side_panel->add_child<ui::VBoxContainer>();
        side_vbox->anchor = ui::Anchor::FILL;
        side_vbox->spacing = 10.0f;

        auto title = side_vbox->add_child<ui::Label>(&text_renderer, "VEHICLE CONFIG");
        title->color = {0.4f, 0.7f, 1.0f, 1.0f};
        title->font_size = 20.0f;
        side_vbox->add_child<ui::Panel>()->size = {280, 1}; // divider

        auto tabs = side_vbox->add_child<TabBar>(&text_renderer);
        tabs->size.y = 40.0f; // Give some height to tab buttons

        // Dedicated container for tab pages to prevent stacking
        auto tab_content_scroll = side_vbox->add_child<ScrollView>();
        tab_content_scroll->expand = true;
        tab_content_scroll->background_color = {0,0,0,0}; // Transparent, just a container
        tab_content_scroll->scroll_v = true;

        auto tab_content = tab_content_scroll->content;

        VBoxContainer* page_vec = tab_content->add_child<VBoxContainer>();
        VBoxContainer* page_phy = tab_content->add_child<VBoxContainer>();
        VBoxContainer* page_env = tab_content->add_child<VBoxContainer>();

        tabs->add_tab("Vehicle", page_vec);
        tabs->add_tab("Physics", page_phy);
        tabs->add_tab("World", page_env);

        // Ensure pages have initial width for child layout
        page_vec->size.x = 290;
        page_phy->size.x = 290;
        page_env->size.x = 290;

        auto add_float_row = [&](Control* page, const std::string& name, float current,
                                  std::function<void(float)> on_commit, float step = 1.0f) {
            auto row = page->add_child<HBoxContainer>();
            row->size = {280, 40}; // Increased height
            row->spacing = 10.0f;
            row->padding = {5, 2, 5, 2};

            auto lbl = row->add_child<Label>(&text_renderer, name);
            lbl->font_size = 14.0f;
            lbl->size.x = 130.0f; // Adjusted width
            lbl->color = {0.9f, 0.9f, 0.9f, 1.0f};

            auto control = row->add_child<Control>();
            control->size.x = 130.0f; // Adjusted width
            control->expand = true;

            auto inp = row->add_child<TextInput>(&text_renderer, "", InputMode::FLOAT);
            inp->size = {150, 32}; // Specified size
            inp->step = step;
            inp->set_float(current, 2);
            inp->font_size = 14.0f;
            inp->color_bg_normal  = {0.18f, 0.18f, 0.25f, 1.0f};
            inp->color_border     = {0.4f, 0.5f, 0.8f, 1.0f};
            inp->on_committed = [on_commit](const std::string& s) {
                try { on_commit(std::stof(s)); } catch (...) {}
            };
        };

        auto add_header = [&](Control* page, const std::string& name, Color color) {
            auto h = page->add_child<Label>(&text_renderer, "--- " + name + " ---");
            h->color = color;
            h->font_size = 14.0f;
            h->margins = {0, 10, 0, 5};
        };

        // --- World Page ---
        add_header(page_env, "TERRAIN", {0.4f, 0.9f, 0.5f, 1.0f});
        add_float_row(page_env, "Bumpiness",  terrain.bumpiness,
            [&terrain](float v){ terrain.bumpiness = std::max(0.0f, v); });
        add_float_row(page_env, "Roughness",  terrain.roughness,
            [&terrain](float v){ terrain.roughness = std::max(0.01f, v); });

        auto btn = page_env->add_child<Button>(&text_renderer, "Generate New Terrain");
        btn->size = {280, 40};
        btn->margins = {0, 15, 0, 0};
        btn->color_normal = {0.15f, 0.4f, 0.2f, 1.0f};
        btn->on_pressed = [&terrain, &car]() {
            terrain.reset(car.get_position().x);
        };

        // --- Vehicle Page ---
        add_header(page_vec, "PERFORMANCE", {0.9f, 0.6f, 0.3f, 1.0f});
        add_float_row(page_vec, "Max Speed", car.max_speed,
            [&car](float v){ car.max_speed = std::max(1.0f, v); });
        add_float_row(page_vec, "Acceleration", car.motor_accel,
            [&car](float v){ car.motor_accel = std::max(1.0f, v); }, 5.0f);
        add_float_row(page_vec, "Air Torque", car.air_torque,
            [&car](float v){ car.air_torque = std::max(0.0f, v); }, 10.0f);
        add_float_row(page_vec, "Air Damping", car.air_damping,
            [&car](float v){ car.air_damping = std::max(0.0f, v); }, 0.1f);
        
        auto reset_btn = page_vec->add_child<Button>(&text_renderer, "Reset Car Position");
        reset_btn->size = {280, 40};
        reset_btn->margins = {0, 5, 0, 15};
        reset_btn->color_normal = {0.4f, 0.3f, 0.2f, 1.0f};
        reset_btn->on_pressed = [&car]() {
            car.reset_car({car.get_position().x, car.get_position().y - 200.0f});
        };
        
        add_header(page_vec, "WHEEL RADIUS", {0.9f, 0.6f, 0.3f, 1.0f});
        add_float_row(page_vec, "Rear Whl", car.r_wheel_radius,
            [&car](float v){ car.r_wheel_radius = std::max(0.1f, v); car.rebuild_physics(); }, 0.05f);
        add_float_row(page_vec, "Front Whl", car.f_wheel_radius,
            [&car](float v){ car.f_wheel_radius = std::max(0.1f, v); car.rebuild_physics(); }, 0.05f);

        add_header(page_vec, "OFFSETS", {0.9f, 0.4f, 0.4f, 1.0f});
        add_float_row(page_vec, "Rear Whl X", car.rear_wheel_x,
            [&car](float v){ car.rear_wheel_x = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_vec, "Rear Whl Y", car.rear_wheel_y,
            [&car](float v){ car.rear_wheel_y = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_vec, "Frt Whl X", car.front_wheel_x,
            [&car](float v){ car.front_wheel_x = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_vec, "Frt Whl Y", car.front_wheel_y,
            [&car](float v){ car.front_wheel_y = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_vec, "Rear Spring", car.r_spring_rest,
            [&car](float v){ car.r_spring_rest = v; car.rebuild_physics(); }, 0.05f);
        add_float_row(page_vec, "Frt Spring", car.f_spring_rest,
            [&car](float v){ car.f_spring_rest = v; car.rebuild_physics(); }, 0.05f);

        // --- Physics Page ---
        add_header(page_phy, "COLLIDER BOUNDS", {0.8f, 0.8f, 0.4f, 1.0f});
        add_float_row(page_phy, "Body Min X", car.body_min_x,
            [&car](float v){ car.body_min_x = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Body Max X", car.body_max_x,
            [&car](float v){ car.body_max_x = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Body Min Y", car.body_min_y,
            [&car](float v){ car.body_min_y = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Body Max Y", car.body_max_y,
            [&car](float v){ car.body_max_y = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Cabin Min X", car.cabin_min_x,
            [&car](float v){ car.cabin_min_x = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Cabin Max X", car.cabin_max_x,
            [&car](float v){ car.cabin_max_x = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Cabin Roof", car.cabin_roof_x,
            [&car](float v){ car.cabin_roof_x = v; car.rebuild_physics(); }, 0.1f);

        add_header(page_phy, "SUSPENSION & MASS", {0.7f, 0.5f, 1.0f, 1.0f});
        add_float_row(page_phy, "Spring Hz", car.susp_hz,
            [&car](float v){ car.susp_hz = std::max(0.1f,v); car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Damping", car.susp_damp,
            [&car](float v){ car.susp_damp = std::clamp(v,0.0f,1.0f); car.rebuild_physics(); }, 0.05f);
        add_float_row(page_phy, "Center X", car.com_x,
            [&car](float v){ car.com_x = v; car.rebuild_physics(); }, 0.1f);
        add_float_row(page_phy, "Center Y", car.com_y,
            [&car](float v){ car.com_y = v; car.rebuild_physics(); }, 0.1f);

        // Camera & View Settings
        add_header(page_env, "CAMERA", {0.4f, 0.8f, 1.0f, 1.0f});
        Camera2D camera(1280.0f, 720.0f);
        camera.config.follow_lerp = 4.0f;
        camera.config.lookahead_dist = 400.0f;
        camera.config.lookahead_lerp = 2.0f;
        camera.set_position({0, 0});

        add_float_row(page_env, "Zoom Level", 0.1f,
            [&camera](float v){ camera.set_zoom(std::max(0.01f, v)); }, 0.05f);
        // Camera and Visuals

        ParticleSystem particles(3000);
        ParticleEmitter smoke;
        smoke.velocity_min = 20.0f;
        smoke.velocity_max = 60.0f;
        smoke.spread_angle = 1.0f; 
        smoke.lifetime_min = 0.4f;
        smoke.lifetime_max = 1.0f;
        smoke.size_start = 30.0f;
        smoke.size_end = 80.0f;
        smoke.color_start = Color(0.8f, 0.8f, 0.8f, 0.5f);
        smoke.color_end = Color(0.5f, 0.5f, 0.5f, 0.0f);
        smoke.drag = 2.5f;
        smoke.mode = EmitterMode::CONTINUOUS;
        smoke.rate = 100.0f;
        smoke.active = true;
        usize smoke_id = particles.add_emitter(smoke);

        int frame_count = 0;
        f32 fps_timer = 0.0f;
        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.should_close()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            f32 delta = std::chrono::duration<f32>(current_time - last_time).count();
            last_time = current_time;

            Vector2i win_size = window.get_size();
            root_hbox->size = {(f32)win_size.x, (f32)win_size.y};
            
            if (Input::is_key_pressed(Key::TAB)) {
                show_ui = !show_ui;
                root_hbox->set_visible(show_ui);
            }

            if (show_ui) {
                root_hbox->layout_children(); // Re-layout for resize
            }

            if (delta > 0.1f) delta = 0.1f;

            Input::update();
            window.poll_events();

            Vector2f mouse_pos = Input::get_mouse_position();
            bool mouse_pressed = Input::is_mouse_pressed(MouseButton::LEFT);
            bool mouse_released = Input::is_mouse_released(MouseButton::LEFT);

            bool ui_handled = root_hbox->update_ui_tree(delta, mouse_pos, mouse_pressed, mouse_released);

            Vector2f world_m;
            world_m.x = camera.get_position().x + (mouse_pos.x - win_size.x / 2.0f) / camera.get_zoom();
            world_m.y = camera.get_position().y + (mouse_pos.y - win_size.y / 2.0f) / camera.get_zoom();
            b2Vec2 p(world_m.x / PIXELS_PER_METER, world_m.y / PIXELS_PER_METER);

            if (mouse_pressed && !ui_handled) { // Only interact with world if UI didn't capture click
                if (mouseJoint) { world.DestroyJoint(mouseJoint); mouseJoint = nullptr; }
                b2AABB aabb;
                b2Vec2 d(0.001f, 0.001f);
                aabb.lowerBound = p - d;
                aabb.upperBound = p + d;
                QueryCallback callback(p);
                world.QueryAABB(&callback, aabb);
                if (callback.fixture) {
                    b2Body* body = callback.fixture->GetBody();
                    b2MouseJointDef md;
                    md.bodyA = groundBody;
                    md.bodyB = body;
                    md.target = p;
                    md.maxForce = 100000.0f * body->GetMass();
                    b2LinearStiffness(md.stiffness, md.damping, 5.0f, 0.7f, groundBody, body);
                    mouseJoint = (b2MouseJoint*)world.CreateJoint(&md);
                    body->SetAwake(true);
                }
            }

            if (mouse_released && mouseJoint) {
                world.DestroyJoint(mouseJoint);
                mouseJoint = nullptr;
            }

            if (mouseJoint) {
                mouseJoint->SetTarget(p);
            }

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            // Handle resize for post-processing
            // pp.resize removed

            // Update Logic
            car.update(delta);
            terrain.update(car.get_position().x); // Generate chunks ahead of the car

            // Pre-physics state
            float prev_speed = car.get_speed();

            // Step Physics
            int32 velocityIterations = 8;
            int32 positionIterations = 3;
            world.Step(delta, velocityIterations, positionIterations);

            // Update Camera and Particles
            float speed = car.get_speed();
            
            // Crash Detection (Hard Deceleration)
            if (prev_speed - speed > 300.0f) {
                float severity = std::min(1.0f, (prev_speed - speed - 300.0f) / 1000.0f);
                camera.add_trauma(0.3f + severity * 0.7f); // Add significant shake based on impact!
            }
            
            bool drive_held = Input::is_key_down(Key::D) || Input::is_key_down(Key::RIGHT) || Input::is_key_down(Key::A) || Input::is_key_down(Key::LEFT);

            Vector2f car_pos = car.get_position();
            Vector2f car_vel = Vector2f{std::cos(car.get_angle()), std::sin(car.get_angle())} * speed;
            if (Input::is_key_down(Key::A) || Input::is_key_down(Key::LEFT)) car_vel.x = -std::abs(car_vel.x);
            else if (drive_held) car_vel.x = std::abs(car_vel.x);

            camera.follow(car_pos, car_vel, delta);
            camera.update(delta);

            auto& em = particles.get_emitter(smoke_id);
            auto angle = car.get_angle();
            // Y is down, X is right. Rotating +y by angle:
            Vector2f down_rel = {-std::sin(angle), std::cos(angle)};
            em.position = car.get_rear_wheel_position() + down_rel * 95.0f; // Bottom of new 0.95m wheel
            if (drive_held && speed > 100.0f) {
                em.rate = 15.0f + speed / 50.0f;
                em.active = true;
                if (Input::is_key_down(Key::A) || Input::is_key_down(Key::LEFT)) {
                    em.rotation = car.get_angle();
                } else {
                    em.rotation = car.get_angle() + 3.14159f;
                }
            } else {
                em.active = false;
            }
            particles.update(delta);

            // Rendering setup
            // win_size already updated at top of loop
            
            // Post-processing scene capture
            // pp.begin_scene removed
            
            // Draw Sky Gradient Fake (clear color)
            glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.init_camera(camera.get_position(), camera.get_zoom(), Vector2f(static_cast<f32>(win_size.x), static_cast<f32>(win_size.y)));

            renderer.begin();

            // Draw Background elements (e.g. distant mountains that move slower via parallax if we had a parallax system)
            // Draw Terrain
            terrain.draw(renderer);
            
            // Draw Car
            car.draw(renderer);
            particles.draw(renderer);

            // Draw loose dynamic obstacles
            for (b2Body* b = world.GetBodyList(); b; b = b->GetNext()) {
                if (b->GetType() == b2_dynamicBody && !car.is_part_of_car(b)) {
                    float body_angle = b->GetAngle();
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

            // Render Scene
            renderer.end();

            // pp.end_scene/render removed

            // Render UI
            if (show_ui) {
                renderer.init_camera({win_size.x / 2.0f, win_size.y / 2.0f}, 1.0f, {(f32)win_size.x, (f32)win_size.y});
                renderer.begin();
                root_hbox->draw_tree(renderer);
                renderer.end();
            }

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

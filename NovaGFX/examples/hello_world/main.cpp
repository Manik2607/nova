#include <nova/nova.hpp>
#include <iostream>
#include <chrono>

using namespace nova;

int main() {
    try {
        Window window(1280, 720, "NovaGFX Benchmark");
        window.set_vsync(false);
        Renderer2D renderer;

        Vector2f sprite_pos(640, 360);
        f32 rotation = 0;
        int frame_count = 0;
        f32 fps_timer = 0.0f;

        auto last_time = std::chrono::high_resolution_clock::now();

        std::vector<Vector2f> poly_points = {
            {0, -50}, {50, 0}, {25, 50}, {-25, 50}, {-50, 0}
        };

        u32 white_data = 0xFFFFFFFF;
        Texture2D default_tex((u8*)&white_data, 1, 1, 4);

        while (!window.should_close()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            f32 delta = std::chrono::duration<f32>(current_time - last_time).count();
            last_time = current_time;

            window.poll_events();
            Input::update();

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            f32 speed = 300.0f;
            if (Input::is_key_down(Key::W)) sprite_pos.y -= speed * delta;
            if (Input::is_key_down(Key::S)) sprite_pos.y += speed * delta;
            if (Input::is_key_down(Key::A)) sprite_pos.x -= speed * delta;
            if (Input::is_key_down(Key::D)) sprite_pos.x += speed * delta;

            rotation += 2.0f * delta;

            Vector2i size = window.get_size();
            glViewport(0, 0, size.x, size.y);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.init_camera(Vector2f(static_cast<f32>(size.x)/2.0f, static_cast<f32>(size.y)/2.0f), 1.0f, Vector2f(static_cast<f32>(size.x), static_cast<f32>(size.y)));

            renderer.begin();

            renderer.draw_rect(Vector2f(0, 0), Vector2f(static_cast<f32>(size.x), static_cast<f32>(size.y)), Color(0.2f, 0.2f, 0.3f));
            renderer.draw_rect(Vector2f(200, 200), Vector2f(100, 100), Color::YELLOW(), rotation, {0.5f, 0.5f});
            renderer.draw_sprite(default_tex, sprite_pos, Vector2f(50, 50), Color::RED(), 0.0f, {0.5f, 0.5f});
            renderer.draw_rect_outline(Vector2f(800, 200), Vector2f(100, 100), Color::GREEN(), 2.0f, -rotation);
            renderer.draw_circle(Vector2f(400, 500), 50.0f, Color::CYAN(), 32);
            renderer.draw_line(Vector2f(100, 600), Vector2f(300, 700), Color::MAGENTA(), 5.0f);

            std::vector<Vector2f> transformed_poly;
            for (auto p : poly_points) transformed_poly.push_back(p + Vector2f(1000, 500));
            renderer.draw_polygon(transformed_poly, Color::ORANGE());

            renderer.end();

            frame_count++;
            fps_timer += delta;
            if (fps_timer >= 1.0f) {
                FrameStats s = renderer.get_stats();
                std::cout << "FPS: " << frame_count << " | Draw calls: " << s.draw_calls << " | Verts: " << s.vertex_count << " | Quads: " << s.quad_count << "\n";
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

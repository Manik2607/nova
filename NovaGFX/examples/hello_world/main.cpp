#include <nova/nova.hpp>
#include <iostream>
#include <chrono>
#include "player.cpp"
using namespace nova;

int main() {
    try {
        Window window(1280, 720, "NovaGFX Benchmark");
        window.set_vsync(false);
        Renderer2D renderer;
        Player player;
        f32 rotation = 0;
        int frame_count = 0;
        f32 fps_timer = 0.0f;

        auto last_time = std::chrono::high_resolution_clock::now();


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
            Vector2i size = window.get_size();
            glViewport(0, 0, size.x, size.y);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.init_camera(Vector2f((size.x)/2.0f, (size.y)/2.0f), 1.0f, Vector2f((size.x), (size.y)));

            renderer.begin();

            player.update(delta);
            player.draw(&renderer);

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

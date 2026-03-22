#include <nova/nova.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <player.hpp>
using namespace nova;

int main() {
    try {
        Window window(1280, 720, "NovaGFX Standalone Example");
        window.set_vsync(true);

        Renderer2D renderer;
        Texture2D player_texture("anime_girl.png");


        Vector2f player_pos(640, 360);
        f32 player_speed = 400.0f;
        Color player_color = Color::CYAN();

        auto last_time = std::chrono::high_resolution_clock::now();

        std::cout << "NovaGFX Standalone Example Started!" << std::endl;
        std::cout << "Controls: W/A/S/D to move, ESC to exit." << std::endl;
        Player player;
        while (!window.should_close()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            f32 delta = std::chrono::duration<f32>(current_time - last_time).count();
            last_time = current_time;

            window.poll_events();
            Input::update();

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            player.update(delta);
            player.draw(renderer);
            Vector2i size = window.get_size();
            glViewport(0, 0, size.x, size.y);
            glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.init_camera(
                Vector2f(static_cast<f32>(size.x) / 2.0f, static_cast<f32>(size.y) / 2.0f),
                1.0f,
                Vector2f(static_cast<f32>(size.x), static_cast<f32>(size.y))
            );

            renderer.begin();

            for (int i = 0; i < size.x; i += 100) {
                renderer.draw_line(Vector2f(static_cast<f32>(i), 0), Vector2f(static_cast<f32>(i), static_cast<f32>(size.y)), Color(0.2f, 0.2f, 0.2f), 1.0f);
            }
            for (int i = 0; i < size.y; i += 100) {
                renderer.draw_line(Vector2f(0, static_cast<f32>(i)), Vector2f(static_cast<f32>(size.x), static_cast<f32>(i)), Color(0.2f, 0.2f, 0.2f), 1.0f);
            }


            renderer.draw_sprite(player_texture, player_pos, Vector2f(300, 300), Color::WHITE(), 0.0f, {0.5f, 0.5f});

            renderer.end();

            window.swap_buffers();
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

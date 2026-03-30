#include <nova/nova.hpp>
#include <iostream>
#include <chrono>
#include "game.hpp"

using namespace nova;

int main() {
    try {
        constexpr i32 WINDOW_W = 720;
        constexpr i32 WINDOW_H = 720/2;

        Window window(WINDOW_W, WINDOW_H, "NEON DRIFT");
        window.set_vsync(true);

        Renderer2D renderer;
        Game game(WINDOW_W, WINDOW_H);

        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.should_close()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            f32 delta = std::chrono::duration<f32>(current_time - last_time).count();
            last_time = current_time;

            // Cap delta to avoid physics explosion
            if (delta > 0.05f) delta = 0.05f;

            Input::update();
            window.poll_events();

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            // Handle resize
            Vector2i size = window.get_size();
            if (size.x > 0 && size.y > 0) {
                game.resize(size.x, size.y);
                glViewport(0, 0, size.x, size.y);
            }

            // Update
            game.update(delta);

            // Draw
            game.draw(renderer);

            window.swap_buffers();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

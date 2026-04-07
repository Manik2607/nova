#include "nova/core.hpp"
#include "nova/platform/window.hpp"
#include "nova/input/input.hpp"
#include "nova/renderer/renderer2d.hpp"
#include "nova/assets/font.hpp"
#include "nova/renderer/text_renderer.hpp"
#include "nova/ui/label.hpp"
#include "nova/ui/text_input.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace nova;

int main() {
    std::cout << "[STEP 2] Starting Font Atlas Generation Test..." << std::endl;

    try {
        Window window(1280, 720, "NovaGFX - Step 2 Font Loading");
        Renderer2D renderer;

        std::string font_path = "C:/data/projects/nova/NovaGFX/examples/font_test/Roboto-VariableFont_wdth,wght.ttf";
        std::cout << "[STEP 2] Attempting to load Font: " << font_path << std::endl;
        
        Font roboto_font(font_path, 64.0f);

        if (roboto_font.get_atlas().get_width() > 1) {
            std::cout << "[STEP 2] SUCCESS! Font atlas created: " 
                      << roboto_font.get_atlas().get_width() << "x" 
                      << roboto_font.get_atlas().get_height() << std::endl;
        } else {
            std::cerr << "[STEP 2] ERROR! Font loading or packing failed. Atlas is 1x1 empty pixel." << std::endl;
        }

        TextRenderer text_renderer;
        nova::ui::Label my_label(&text_renderer, "High Score: 9,001");
        nova::ui::TextInput input(&text_renderer,"hello");
        my_label.font_override = &roboto_font; 
        my_label.font_size = 64.0f;
        my_label.position = {200.0f, 400.0f};

        while (!window.should_close()) {
            Input::update();
            window.poll_events();

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            // Dark background
            glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.begin();
            renderer.init_camera({640.0f, 360.0f}, 1.0f, {1280.0f, 720.0f});

            my_label.draw(renderer);

            input.position = {200.0f, 200.0f};
            input.size = {300.0f, 50.0f};
            input.font_override = &roboto_font;
            input.font_size = 32.0f;
            input.draw(renderer);

            renderer.end();
            window.swap_buffers();
        }

    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL ERROR] " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[STEP 2] Exiting cleanly." << std::endl;
    return 0;
}

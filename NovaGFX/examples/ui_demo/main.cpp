#include "nova/core.hpp"
#include "nova/platform/window.hpp"
#include "nova/input/input.hpp"
#include "nova/renderer/renderer2d.hpp"
#include "nova/renderer/text_renderer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "nova/ui/control.hpp"
#include "nova/ui/label.hpp"
#include "nova/ui/button.hpp"
#include "nova/ui/text_input.hpp"

using namespace nova;
using namespace nova::ui;

int main() {
    Window window(1280, 720, "NovaGFX UI Demo");
    Renderer2D renderer;
    TextRenderer text_renderer;

    std::unique_ptr<Control> root_ui = std::make_unique<Control>();
    root_ui->size = {1280.0f, 720.0f};

    // Create a Label
    auto label = std::make_unique<Label>(&text_renderer, "Welcome to Nova UI");
    label->position = {400.0f, 100.0f};
    label->font_size = 32.0f;
    label->color = {0.0f, 1.0f, 1.0f, 1.0f}; // Cyan text
    
    // Store raw ptr so button can change its text
    Label* label_ptr = label.get();
    root_ui->add_child(std::move(label));

    // Create a Button
    auto btn = std::make_unique<Button>(&text_renderer, "Click Me!");
    btn->position = {500.0f, 200.0f};
    btn->size = {200.0f, 60.0f};
    btn->font_size = 24.0f;
    btn->color_normal = {0.1f, 0.4f, 0.6f, 1.0f};
    btn->color_hover = {0.2f, 0.5f, 0.7f, 1.0f};
    btn->color_pressed = {0.05f, 0.3f, 0.5f, 1.0f};
    
    int click_count = 0;
    btn->on_pressed = [&click_count, label_ptr]() {
        click_count++;
        label_ptr->text = "Button clicked " + std::to_string(click_count) + " times!";
    };
    root_ui->add_child(std::move(btn));

    // Create a TextInput
    auto input = std::make_unique<TextInput>(&text_renderer, "Type here");
    input->position = {500.0f, 300.0f};
    input->size = {280.0f, 50.0f};
    input->font_size = 24.0f;
    root_ui->add_child(std::move(input));

    f32 last_time = 0.0f;

    while (!window.should_close()) {
        f32 current_time = static_cast<f32>(glfwGetTime());
        f32 delta = current_time - last_time;
        last_time = current_time;

        Input::update();
        window.poll_events();

        if (Input::is_key_pressed(Key::ESCAPE)) {
            window.close();
        }

        Vector2f mouse_pos = Input::get_mouse_position();
        bool mouse_pressed = Input::is_mouse_pressed(MouseButton::LEFT);
        bool mouse_released = Input::is_mouse_released(MouseButton::LEFT);

        root_ui->update_ui_tree(delta, mouse_pos, mouse_pressed, mouse_released);

        // Rendering
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer.begin();
        renderer.init_camera({window.get_size().x / 2.0f, window.get_size().y / 2.0f}, 1.0f, {(f32)window.get_size().x, (f32)window.get_size().y});
        
        root_ui->draw_tree(renderer);
        renderer.end();

        window.swap_buffers();
    }

    return 0;
}

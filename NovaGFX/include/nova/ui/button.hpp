#pragma once
#include "nova/ui/control.hpp"
#include "nova/renderer/text_renderer.hpp"
#include <string>
#include <functional>

namespace nova::ui {

class Button : public Control {
public:
    std::string text;
    f32 font_size{24.0f};
    
    Color color_normal{0.2f, 0.2f, 0.2f, 1.0f};
    Color color_hover{0.3f, 0.3f, 0.3f, 1.0f};
    Color color_pressed{0.1f, 0.1f, 0.1f, 1.0f};
    Color color_text{Color::WHITE()};

    TextRenderer* text_renderer{nullptr};
    const Font* font_override{nullptr};
    std::function<void()> on_pressed;

    Button(TextRenderer* tr, std::string_view initial_text = "")
        : text(initial_text), text_renderer(tr) {
        size = {120.0f, 40.0f}; // default size
    }

    void draw(Renderer2D& renderer) override;
    bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) override;

private:
    bool m_is_pressed_internal{false};
};

} // namespace nova::ui

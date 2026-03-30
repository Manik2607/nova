#pragma once
#include "nova/ui/control.hpp"
#include "nova/renderer/text_renderer.hpp"
#include <string>

namespace nova::ui {

class TextInput : public Control {
public:
    std::string text;
    f32 font_size{24.0f};

    Color color_bg_normal{0.1f, 0.1f, 0.1f, 1.0f};
    Color color_bg_focused{0.2f, 0.2f, 0.2f, 1.0f};
    Color color_border{0.5f, 0.5f, 0.5f, 1.0f};
    Color color_text{Color::WHITE()};

    TextRenderer* text_renderer{nullptr};

    TextInput(TextRenderer* tr, std::string_view initial_text = "")
        : text_renderer(tr), text(initial_text) {
        size = {200.0f, 40.0f};
    }

    void draw(Renderer2D& renderer) override;
    void update(f32 delta) override;
    bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) override;

private:
    f32 m_blink_timer{0.0f};
    bool m_show_caret{true};
    
    void handle_keyboard_input();
};

} // namespace nova::ui

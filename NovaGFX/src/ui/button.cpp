#include "nova/ui/button.hpp"

namespace nova::ui {

void Button::draw(Renderer2D& renderer) {
    Transform2D global = get_global_transform();
    
    Color current_color = color_normal;
    if (m_is_pressed_internal) {
        current_color = color_pressed;
    } else if (is_hovered) {
        current_color = color_hover;
    }

    renderer.draw_rect(global.origin, size, current_color);

    if (text_renderer && !text.empty()) {
        f32 text_width = 0.0f;
        if (font_override) {
            text_width = text_renderer->measure_text_with_font(*font_override, text, font_size);
        } else {
            text_width = text_renderer->measure_text(text, font_size);
        }
        
        // Center text box vertically by setting the baseline at the vertical middle + half font height
        // This is a more robust way to center text of different fonts.
        Vector2f text_pos = global.origin + size * 0.5f - Vector2f{text_width * 0.5f, -font_size * 0.35f};
        
        if (font_override) {
            text_renderer->draw_text_with_font(renderer, *font_override, text, text_pos, font_size, color_text);
        } else {
            text_renderer->draw_text(renderer, text, text_pos, font_size, color_text);
        }
    }
}

bool Button::handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) {
    // removed was_hovered
    is_hovered = get_global_rect().contains(mouse_pos);

    if (is_hovered) {
        if (mouse_pressed) {
            m_is_pressed_internal = true;
            return true;
        }
        if (mouse_released && m_is_pressed_internal) {
            if (on_pressed) on_pressed();
            m_is_pressed_internal = false;
            return true;
        }
    } else {
        if (mouse_released) {
            m_is_pressed_internal = false;
        }
    }
    
    return is_hovered;
}

} // namespace nova::ui

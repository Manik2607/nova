#include "nova/ui/text_input.hpp"
#include "nova/input/input.hpp"
#include "nova/input/keyboard.hpp"

namespace nova::ui {

void TextInput::draw(Renderer2D& renderer) {
    Transform2D global = get_global_transform();
    
    // Draw background
    Color bg = is_focused ? color_bg_focused : color_bg_normal;
    renderer.draw_rect(global.origin, size, bg);
    renderer.draw_rect_outline(global.origin, size, color_border, 2.0f);

    Vector2f text_pos = global.origin + Vector2f{10.0f, (size.y - font_size) * 0.5f};
    
    f32 text_width = 0.0f;
    if (text_renderer) {
        if (!text.empty()) {
            text_renderer->draw_text(renderer, text, text_pos, font_size, color_text);
            text_width = text_renderer->measure_text(text, font_size);
        }

        // Draw caret
        if (is_focused && m_show_caret) {
            Vector2f caret_pos = text_pos + Vector2f{text_width + 4.0f, 0.0f};
            renderer.draw_rect(caret_pos, {2.0f, font_size}, color_text);
        }
    }
}

void TextInput::update(f32 delta) {
    if (is_focused) {
        m_blink_timer += delta;
        if (m_blink_timer > 0.5f) {
            m_show_caret = !m_show_caret;
            m_blink_timer -= 0.5f;
        }

        handle_keyboard_input();
    } else {
        m_show_caret = false;
        m_blink_timer = 0.0f;
    }
}

bool TextInput::handle_input(Vector2f mouse_pos, bool mouse_pressed, bool /*mouse_released*/) {
    is_hovered = get_global_rect().contains(mouse_pos);
    
    if (mouse_pressed) {
        if (is_hovered) {
            is_focused = true;
            m_show_caret = true;
            m_blink_timer = 0.0f;
            return true;
        } else {
            is_focused = false;
        }
    }
    
    return is_hovered;
}

void TextInput::handle_keyboard_input() {
    // Backspace
    if (Input::is_key_pressed(Key::BACKSPACE) && !text.empty()) {
        text.pop_back();
    }
    // Space
    if (Input::is_key_pressed(Key::SPACE)) {
        text += ' ';
    }
    
    // Alphanumeric - simple map since Nova doesn't have character callbacks exposed yet
    for (int k = (int)Key::A; k <= (int)Key::Z; ++k) {
        if (Input::is_key_pressed(static_cast<Key>(k))) {
            char base = Input::is_key_down(Key::LEFT_SHIFT) || Input::is_key_down(Key::RIGHT_SHIFT) ? 'A' : 'a';
            text += static_cast<char>(base + (k - (int)Key::A));
        }
    }
    
    for (int k = (int)Key::NUM_0; k <= (int)Key::NUM_9; ++k) {
        if (Input::is_key_pressed(static_cast<Key>(k))) {
            text += static_cast<char>('0' + (k - (int)Key::NUM_0));
        }
    }
}

} // namespace nova::ui

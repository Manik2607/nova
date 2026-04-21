#include "nova/ui/text_input.hpp"
#include "nova/input/input.hpp"
#include "nova/input/keyboard.hpp"
#include <algorithm>
#include <cmath>

namespace nova::ui {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void TextInput::clamp_cursor() {
    m_cursor = std::clamp(m_cursor, 0, static_cast<int>(text.size()));
}

void TextInput::insert_char(char c) {
    clamp_cursor();
    text.insert(text.begin() + m_cursor, c);
    ++m_cursor;
}

void TextInput::apply_step(float delta) {
    if (mode == InputMode::INT) {
        int v = get_int(0);
        v += static_cast<int>(delta);
        set_int(v);
    } else if (mode == InputMode::FLOAT) {
        float v = get_float(0.0f);
        v += delta;
        // Keep existing decimal count (at most 6 places)
        size_t dot = text.find('.');
        int decimals = (dot == std::string::npos) ? 2 : static_cast<int>(text.size() - dot - 1);
        decimals = std::clamp(decimals, 1, 6);
        set_float(v, decimals);
    }
    if (on_committed) on_committed(text);
}

// ---------------------------------------------------------------------------
// draw
// ---------------------------------------------------------------------------

void TextInput::draw(Renderer2D& renderer) {
    Transform2D global = get_global_transform();

    f32 text_box_w = size.x;
    if (is_numeric()) {
        text_box_w -= BTN_W * 2;
    }
    if (text_box_w < 0.0f) text_box_w = 0.0f;

    // --- Background ---
    Color bg = is_focused ? color_bg_focused : color_bg_normal;
    renderer.draw_rect(global.origin, {text_box_w, size.y}, bg);
    renderer.draw_rect_outline(global.origin, {text_box_w, size.y}, color_border, 2.0f);

    // --- Text + caret ---
    if (text_renderer) {
        clamp_cursor();

        auto draw_txt = [&](std::string_view str, Vector2f pos, f32 fs, Color col) {
            if (font_override) text_renderer->draw_text_with_font(renderer, *font_override, str, pos, fs, col);
            else text_renderer->draw_text(renderer, str, pos, fs, col);
        };
        auto measure_txt = [&](std::string_view str, f32 fs) {
            if (font_override) return text_renderer->measure_text_with_font(*font_override, str, fs);
            return text_renderer->measure_text(str, fs);
        };

        // --- Step buttons (only for numeric modes) ---
        const f32 btn_h = size.y;
        // Buttons sit to the right of the text field
        Vector2f btn_base = global.origin + Vector2f{text_box_w, 0.0f};

        if (is_numeric()) {
            // Decrease (–) button
            Color dec_col = m_btn_dec_pressed ? color_btn_pressed
                          : (m_btn_dec_hovered ? color_btn_hover : color_btn_normal);
            renderer.draw_rect(btn_base, {BTN_W, btn_h}, dec_col);
            renderer.draw_rect_outline(btn_base, {BTN_W, btn_h}, color_border, 1.0f);
            f32 lbl_w = measure_txt("-", font_size);
            draw_txt("-", btn_base + Vector2f{(BTN_W - lbl_w) * 0.5f, (btn_h + font_size) * 0.40f},
                     font_size, color_text);

            // Increase (+) button
            Vector2f inc_pos = btn_base + Vector2f{BTN_W, 0.0f};
            Color inc_col = m_btn_inc_pressed ? color_btn_pressed
                          : (m_btn_inc_hovered ? color_btn_hover : color_btn_normal);
            renderer.draw_rect(inc_pos, {BTN_W, btn_h}, inc_col);
            renderer.draw_rect_outline(inc_pos, {BTN_W, btn_h}, color_border, 1.0f);
            lbl_w = measure_txt("+", font_size);
            draw_txt("+", inc_pos + Vector2f{(BTN_W - lbl_w) * 0.5f, (btn_h + font_size) * 0.40f},
                     font_size, color_text);
        }

        // Measure text before cursor to position the caret
        std::string before_cursor = text.substr(0, static_cast<size_t>(m_cursor));
        f32 caret_x = measure_txt(before_cursor.empty() ? "" : before_cursor, font_size);

        // Baseline for centered text in the input box
        Vector2f text_pos = global.origin + Vector2f{10.0f, size.y * 0.5f + font_size * 0.35f};

        if (!text.empty()) {
            draw_txt(text, text_pos, font_size, color_text);
        }

        // Caret
        if (is_focused && m_show_caret) {
            // Center caret vertically
            Vector2f caret_pos = global.origin + Vector2f{10.0f + caret_x, (size.y - font_size) * 0.5f};
            renderer.draw_rect(caret_pos, {2.0f, font_size}, color_text);
        }
    }
}

// ---------------------------------------------------------------------------
// update
// ---------------------------------------------------------------------------

void TextInput::update(f32 delta) {
    if (is_focused) {
        m_blink_timer += delta;
        if (m_blink_timer > 0.5f) {
            m_show_caret = !m_show_caret;
            m_blink_timer -= 0.5f;
        }
        handle_keyboard_input();

        // Handle Backspace repeating
        if (Input::is_key_down(Key::BACKSPACE)) {
            m_backspace_timer += delta;
            if (m_backspace_timer >= 0.05f) { // Deletion rate
                clamp_cursor();
                if (m_cursor > 0 && !text.empty()) {
                    --m_cursor;
                    text.erase(text.begin() + m_cursor);
                    m_show_caret = true;
                    m_blink_timer = 0.0f;
                }
                m_backspace_timer = 0.0f;
            }
        }
    } else {
        m_show_caret = false;
        m_blink_timer = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// handle_input (mouse)
// ---------------------------------------------------------------------------

bool TextInput::handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) {
    is_hovered = get_global_rect().contains(mouse_pos);

    f32 text_box_w = size.x;
    if (is_numeric()) {
        text_box_w -= BTN_W * 2;
    }
    if (text_box_w < 0.0f) text_box_w = 0.0f;

    // --- Step button hit detection ---
    bool consumed = false;
    if (is_numeric()) {
        Transform2D global = get_global_transform();
        Vector2f btn_base = global.origin + Vector2f{text_box_w, 0.0f};
        Rect2f dec_rect{btn_base, {BTN_W, size.y}};
        Rect2f inc_rect{btn_base + Vector2f{BTN_W, 0.0f}, {BTN_W, size.y}};

        m_btn_dec_hovered = dec_rect.contains(mouse_pos);
        m_btn_inc_hovered = inc_rect.contains(mouse_pos);

        if (mouse_pressed) {
            if (m_btn_dec_hovered) { m_btn_dec_pressed = true; consumed = true; }
            if (m_btn_inc_hovered) { m_btn_inc_pressed = true; consumed = true; }
        }
        if (mouse_released) {
            if (m_btn_dec_pressed && m_btn_dec_hovered) { apply_step(-step); consumed = true; }
            if (m_btn_inc_pressed && m_btn_inc_hovered) { apply_step( step); consumed = true; }
            m_btn_dec_pressed = false;
            m_btn_inc_pressed = false;
        }
    }

    // --- Focus management ---
    if (mouse_pressed) {
        Rect2f text_rect{get_global_transform().origin, {text_box_w, size.y}};
        bool text_hovered = text_rect.contains(mouse_pos);

        if (text_hovered) {
            is_focused = true;
            m_show_caret = true;
            m_blink_timer = 0.0f;
            // Place cursor at end on click (simple approach)
            m_cursor = static_cast<int>(text.size());
            return true;
        } else if (!consumed) {
            is_focused = false;
        }
    }

    return is_hovered || consumed;
}

// ---------------------------------------------------------------------------
// handle_keyboard_input
// ---------------------------------------------------------------------------

void TextInput::handle_keyboard_input() {
    bool shift = Input::is_key_down(Key::LEFT_SHIFT) || Input::is_key_down(Key::RIGHT_SHIFT);

    // Enter / commit
    if (Input::is_key_pressed(Key::ENTER) || Input::is_key_pressed(Key::KP_ENTER)) {
        is_focused = false;
        if (on_committed) on_committed(text);
        return;
    }

    // Escape – unfocus without committing
    if (Input::is_key_pressed(Key::ESCAPE)) {
        is_focused = false;
        return;
    }

    // Cursor movement
    if (Input::is_key_pressed(Key::LEFT)) {
        if (m_cursor > 0) { --m_cursor; m_show_caret = true; m_blink_timer = 0.0f; }
        return;
    }
    if (Input::is_key_pressed(Key::RIGHT)) {
        if (m_cursor < static_cast<int>(text.size())) { ++m_cursor; m_show_caret = true; m_blink_timer = 0.0f; }
        return;
    }
    if (Input::is_key_pressed(Key::HOME)) {
        m_cursor = 0; m_show_caret = true; m_blink_timer = 0.0f;
        return;
    }
    if (Input::is_key_pressed(Key::END)) {
        m_cursor = static_cast<int>(text.size()); m_show_caret = true; m_blink_timer = 0.0f;
        return;
    }

    // Delete (delete char at cursor)
    if (Input::is_key_pressed(Key::DELETE_KEY)) {
        clamp_cursor();
        if (m_cursor < static_cast<int>(text.size())) {
            text.erase(text.begin() + m_cursor);
        }
        return;
    }

    // Backspace (initial press)
    if (Input::is_key_pressed(Key::BACKSPACE)) {
        clamp_cursor();
        if (m_cursor > 0 && !text.empty()) {
            --m_cursor;
            text.erase(text.begin() + m_cursor);
            m_show_caret = true;
            m_blink_timer = 0.0f;
        }
        m_backspace_timer = -0.4f; // Initial delay before repeating starts
        return;
    }

    // ------------------------------------------------------------------
    // Character insertion (mode-aware)
    // ------------------------------------------------------------------
    auto try_insert = [&](char c) {
        switch (mode) {
            case InputMode::TEXT:
                insert_char(c);
                break;
            case InputMode::INT:
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    insert_char(c);
                } else if (c == '-' && m_cursor == 0 && (text.empty() || text[0] != '-')) {
                    insert_char(c);
                }
                break;
            case InputMode::FLOAT:
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    insert_char(c);
                } else if ((c == '.' || c == ',') && text.find('.') == std::string::npos) {
                    insert_char('.');
                } else if (c == '-' && m_cursor == 0 && (text.empty() || text[0] != '-')) {
                    insert_char(c);
                }
                break;
        }
    };

    // Period / decimal (also KP_DECIMAL)
    if (Input::is_key_pressed(Key::PERIOD) || Input::is_key_pressed(Key::KP_DECIMAL)) {
        if (mode == InputMode::FLOAT && text.find('.') == std::string::npos) {
            insert_char('.');
        } else if (mode == InputMode::TEXT) {
            insert_char('.');
        }
        return;
    }

    // Minus sign (row & numpad)
    if (Input::is_key_pressed(Key::MINUS) || Input::is_key_pressed(Key::KP_SUBTRACT)) {
        if (mode == InputMode::TEXT) {
            insert_char('-');
        } else if ((mode == InputMode::INT || mode == InputMode::FLOAT)
                   && m_cursor == 0 && (text.empty() || text[0] != '-')) {
            insert_char('-');
        }
        return;
    }

    // Space (TEXT only)
    if (mode == InputMode::TEXT && Input::is_key_pressed(Key::SPACE)) {
        insert_char(' ');
        return;
    }

    // Alphanumeric row letters (TEXT only)
    if (mode == InputMode::TEXT) {
        for (int k = static_cast<int>(Key::A); k <= static_cast<int>(Key::Z); ++k) {
            if (Input::is_key_pressed(static_cast<Key>(k))) {
                char base = shift ? 'A' : 'a';
                insert_char(static_cast<char>(base + (k - static_cast<int>(Key::A))));
            }
        }
    }

    // Row digit keys (0–9)
    for (int k = static_cast<int>(Key::NUM_0); k <= static_cast<int>(Key::NUM_9); ++k) {
        if (Input::is_key_pressed(static_cast<Key>(k))) {
            try_insert(static_cast<char>('0' + (k - static_cast<int>(Key::NUM_0))));
        }
    }

    // Numpad digit keys (KP_0–KP_9)
    for (int k = static_cast<int>(Key::KP_0); k <= static_cast<int>(Key::KP_9); ++k) {
        if (Input::is_key_pressed(static_cast<Key>(k))) {
            try_insert(static_cast<char>('0' + (k - static_cast<int>(Key::KP_0))));
        }
    }
}

} // namespace nova::ui

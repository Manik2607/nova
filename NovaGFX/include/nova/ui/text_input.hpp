#pragma once
#include "nova/ui/control.hpp"
#include "nova/renderer/text_renderer.hpp"
#include <string>
#include <functional>
#include <cstdio>

namespace nova::ui {

/// Determines what kind of value this TextInput accepts and how it behaves.
enum class InputMode {
    TEXT,   ///< Free-form text (any printable characters)
    INT,    ///< Integer digits + minus sign; shows +/- step buttons
    FLOAT,  ///< Float digits + minus + decimal; shows +/- step buttons
};

class TextInput : public Control {
public:
    std::string text;
    f32 font_size{24.0f};

    Color color_bg_normal{0.1f, 0.1f, 0.1f, 1.0f};
    Color color_bg_focused{0.2f, 0.2f, 0.2f, 1.0f};
    Color color_border{0.5f, 0.5f, 0.5f, 1.0f};
    Color color_text{Color::WHITE()};

    /// Colors for the +/- step buttons
    Color color_btn_normal{0.25f, 0.25f, 0.25f, 1.0f};
    Color color_btn_hover{0.40f, 0.40f, 0.40f, 1.0f};
    Color color_btn_pressed{0.12f, 0.12f, 0.12f, 1.0f};

    TextRenderer* text_renderer{nullptr};

    /// Mode controlling accepted input and whether step buttons appear.
    InputMode mode{InputMode::TEXT};

    /// Step amount applied by +/- buttons (only used for INT/FLOAT mode).
    float step{1.0f};

    TextInput(TextRenderer* tr, std::string_view initial_text = "", InputMode m = InputMode::TEXT)
        : text_renderer(tr), text(initial_text), mode(m) {
        size = {200.0f, 40.0f};
    }

    std::function<void(const std::string&)> on_committed;

    // -----------------------------------------------------------------------
    // Convenience value accessors
    // -----------------------------------------------------------------------
    float get_float(float fallback = 0.0f) const {
        try { return std::stof(text); } catch (...) { return fallback; }
    }
    int get_int(int fallback = 0) const {
        try { return std::stoi(text); } catch (...) { return fallback; }
    }
    void set_float(float v, int decimals = 2) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.*f", decimals, v);
        text = buf;
        m_cursor = static_cast<int>(text.size());
    }
    void set_int(int v) {
        text = std::to_string(v);
        m_cursor = static_cast<int>(text.size());
    }

    void draw(Renderer2D& renderer) override;
    void update(f32 delta) override;
    bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) override;

private:
    f32  m_blink_timer{0.0f};
    bool m_show_caret{true};
    int  m_cursor{0};   ///< Caret position (index into text, 0 = before first char)

    // Step button hit state
    bool m_btn_inc_hovered{false};
    bool m_btn_dec_hovered{false};
    bool m_btn_inc_pressed{false};
    bool m_btn_dec_pressed{false};

    static constexpr f32 BTN_W = 22.0f;  ///< Width of each step button

    /// Returns true if mode requires numeric step buttons.
    bool is_numeric() const { return mode == InputMode::INT || mode == InputMode::FLOAT; }

    void clamp_cursor();
    void insert_char(char c);
    void handle_keyboard_input();
    void apply_step(float delta);
};

} // namespace nova::ui

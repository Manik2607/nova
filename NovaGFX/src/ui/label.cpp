#include "nova/ui/label.hpp"

namespace nova::ui {

void Label::draw(Renderer2D& renderer) {
    if (text_renderer && !text.empty()) {
        Transform2D global = get_global_transform();
        if (font_override) {
            text_renderer->draw_text_with_font(renderer, *font_override, text, global.origin, font_size, color);
        } else {
            text_renderer->draw_text(renderer, text, global.origin + Vector2f{0.0f, font_size}, font_size, color);
        }
    }
}

void Label::update_size() {
    if (text_renderer) {
        if (font_override) {
            size.x = text_renderer->measure_text_with_font(*font_override, text, font_size);
        } else {
            size.x = text_renderer->measure_text(text, font_size);
        }
        size.y = font_size; // simple approximation
    }
}

} // namespace nova::ui

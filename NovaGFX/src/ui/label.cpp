#include "nova/ui/label.hpp"

namespace nova::ui {

void Label::draw(Renderer2D& renderer) {
    if (text_renderer && !text.empty()) {
        Transform2D global = get_global_transform();
        text_renderer->draw_text(renderer, text, global.origin, font_size, color);
    }
}

void Label::update_size() {
    if (text_renderer) {
        size.x = text_renderer->measure_text(text, font_size);
        size.y = font_size; // simple approximation
    }
}

} // namespace nova::ui

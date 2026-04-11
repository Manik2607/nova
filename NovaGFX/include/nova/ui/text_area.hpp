#pragma once
#include "nova/ui/control.hpp"
#include "nova/renderer/text_renderer.hpp"
#include <vector>
#include <string>

namespace nova::ui {

class TextArea : public Control {
public:
    std::string text;
    f32 font_size{18.0f};
    Color color_text{Color::WHITE()};
    Color color_bg{0.05f, 0.05f, 0.05f, 0.9f};
    
    TextRenderer* text_renderer{nullptr};

    TextArea(TextRenderer* tr, std::string_view initial_text = "") 
        : text(initial_text), text_renderer(tr) {
        size = {300, 150};
    }

    void draw(Renderer2D& renderer) override {
        Rect2f rect = get_global_rect();
        renderer.draw_rect(rect.position, rect.size, color_bg);
        renderer.draw_rect_outline(rect.position, rect.size, {0.3f, 0.3f, 0.3f, 1.0f}, 1.0f);
        
        if (text_renderer) {
            // Split by newline and draw
            f32 y_offset = 10.0f;
            size_t start = 0;
            size_t end = text.find('\n');
            while (end != std::string::npos) {
                std::string_view line = std::string_view(text).substr(start, end - start);
                text_renderer->draw_text(renderer, line, rect.position + Vector2f(10, y_offset + font_size), font_size, color_text);
                y_offset += font_size + 4.0f;
                start = end + 1;
                end = text.find('\n', start);
            }
            // Last line
            std::string_view line = std::string_view(text).substr(start);
            text_renderer->draw_text(renderer, line, rect.position + Vector2f(10, y_offset + font_size), font_size, color_text);
        }
    }
    
    void append_text(std::string_view t) {
        text += t;
    }
};

} // namespace nova::ui

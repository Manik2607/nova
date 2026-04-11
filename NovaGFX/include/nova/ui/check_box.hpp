#pragma once
#include "nova/ui/control.hpp"
#include "nova/ui/label.hpp"

namespace nova::ui {

class CheckBox : public Control {
public:
    std::string text;
    bool checked{false};
    std::function<void(bool)> on_changed;
    
    Color color_box{0.2f, 0.2f, 0.2f, 1.0f};
    Color color_check{0.4f, 0.9f, 0.5f, 1.0f};
    
    TextRenderer* text_renderer{nullptr};
    f32 font_size{18.0f};

    CheckBox(TextRenderer* tr, std::string_view label_text = "") 
        : text(label_text), text_renderer(tr) {
        size = {120, 24};
    }

    void draw(Renderer2D& renderer) override {
        Rect2f rect = get_global_rect();
        f32 box_size = rect.size.y;
        
        // Draw box
        renderer.draw_rect(rect.position, {box_size, box_size}, color_box);
        renderer.draw_rect_outline(rect.position, {box_size, box_size}, Color::WHITE(), 1.0f);
        
        if (checked) {
            f32 p = box_size * 0.2f;
            renderer.draw_rect(rect.position + Vector2f(p, p), {box_size - p*2, box_size - p*2}, color_check);
        }
        
        // Draw label
        if (text_renderer) {
            // Label draw logic (ideally we use a Label child, but this is fine for now)
            // For now, I'll just rely on the user using add_child<Label> if I want to be purely clean, 
            // but for "minimal lines" let's just make it work.
            // Actually, let's use a child Label to avoid duplicating logic.
            if (m_children.empty()) {
                auto lbl = std::make_unique<Label>(text_renderer, text);
                lbl->position = {box_size + 8.0f, 0.0f};
                lbl->font_size = font_size;
                child_label = lbl.get();
                add_child(std::move(lbl));
            }
            child_label->text = text;
            child_label->font_size = font_size;
        }
    }

    bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool /*mouse_released*/) override {
        bool hovered = get_global_rect().contains(mouse_pos);
        if (hovered && mouse_pressed) {
            checked = !checked;
            if (on_changed) on_changed(checked);
            return true;
        }
        return false;
    }

private:
    Label* child_label{nullptr};
};

} // namespace nova::ui

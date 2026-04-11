#pragma once
#include "nova/ui/control.hpp"

namespace nova::ui {

class Slider : public Control {
public:
    f32 value{0.5f}; // 0.0 to 1.0
    f32 min_val{0.0f};
    f32 max_val{1.0f};
    
    std::function<void(f32)> on_changed;
    
    Color color_track{0.2f, 0.2f, 0.2f, 1.0f};
    Color color_grabber{0.4f, 0.6f, 1.0f, 1.0f};
    
    Slider() {
        size = {150, 20};
    }

    void draw(Renderer2D& renderer) override {
        Rect2f rect = get_global_rect();
        f32 track_h = 4.0f;
        
        // Track
        renderer.draw_rect(rect.position + Vector2f(0, rect.size.y * 0.5f - track_h * 0.5f), 
                           {rect.size.x, track_h}, color_track);
        
        // Grabber
        f32 pos_x = (value - min_val) / (max_val - min_val) * rect.size.x;
        f32 grabber_w = 12.0f;
        renderer.draw_rect(rect.position + Vector2f(pos_x - grabber_w * 0.5f, 0), 
                           {grabber_w, rect.size.y}, color_grabber);
    }

    bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool /*mouse_released*/) override {
        Rect2f rect = get_global_rect();
        bool hovered = rect.contains(mouse_pos);
        
        if (hovered && mouse_pressed) {
            m_dragging = true;
        }
        
        if (m_dragging) {
            if (!Input::is_mouse_down(MouseButton::LEFT)) {
                m_dragging = false;
            } else {
                f32 local_x = mouse_pos.x - rect.position.x;
                f32 pct = std::clamp(local_x / rect.size.x, 0.0f, 1.0f);
                value = min_val + pct * (max_val - min_val);
                if (on_changed) on_changed(value);
            }
            return true;
        }
        return false;
    }

private:
    bool m_dragging{false};
};

} // namespace nova::ui

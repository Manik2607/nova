#pragma once
#include "nova/ui/control.hpp"
#include "nova/input/input.hpp"
#include <cmath>

namespace nova::ui {

class ScrollView : public Control {
public:
    Vector2f scroll_pos{0, 0};
    bool scroll_h{true};
    bool scroll_v{true};
    
    Control* content{nullptr};

    ScrollView() {
        size = {200, 200};
        // ScrollView usually has a 'content' node
        content = add_child<Control>();
    }
    void draw_tree(Renderer2D& renderer) override {
        if (!visible) return;
        Rect2f rect = get_global_rect();
        
        // Use custom draw with clipping
        renderer.set_clip_rect(rect);
        
        // Draw background
        renderer.draw_rect(rect.position, rect.size, {0.1f, 0.1f, 0.1f, 1.0f});
        
        // We need to offset the drawing of children by scroll_pos
        // However, Node2D uses local positions. We can just set content position.
        if (content) {
            content->position = -scroll_pos;
        }
        
        // Draw children (wrapped in clipping)
        draw(renderer);
        for (auto& child : m_children) {
            child->draw_tree(renderer);
        }
        
        renderer.clear_clip_rect();
    }

    bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) override {
        is_hovered = get_global_rect().contains(mouse_pos);
        return false;
    }

    void update(f32 /*delta*/) override {
        if (!is_hovered) return;

        // Mouse wheel scroll
        Vector2f scroll = Input::get_scroll_delta();
        if (std::abs(scroll.y) > 0.1f) {
            scroll_pos.y -= scroll.y * 30.0f; // Scroll speed
            
            // Clamping
            if (content) {
                f32 max_scroll = std::max(0.0f, content->size.y - size.y);
                if (scroll_pos.y < 0) scroll_pos.y = 0;
                if (scroll_pos.y > max_scroll) scroll_pos.y = max_scroll;
            }
        }
    }
    
    // Helper to add to content instead of the ScrollView itself
    template<typename T, typename... Args>
    T* add_to_content(Args&&... args) {
        return content->add_child<T>(std::forward<Args>(args)...);
    }
};

} // namespace nova::ui

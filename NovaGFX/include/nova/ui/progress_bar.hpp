#pragma once
#include "nova/ui/control.hpp"

namespace nova::ui {

class ProgressBar : public Control {
public:
    f32 value{0.0f}; // 0.0 to 1.0
    Color color_bg{0.1f, 0.1f, 0.1f, 1.0f};
    Color color_fill{0.2f, 0.8f, 0.3f, 1.0f};
    
    ProgressBar() {
        size = {150, 20};
    }

    void draw(Renderer2D& renderer) override {
        Rect2f rect = get_global_rect();
        renderer.draw_rect(rect.position, rect.size, color_bg);
        
        f32 fill_width = size.x * std::clamp(value, 0.0f, 1.0f);
        if (fill_width > 0) {
            renderer.draw_rect(rect.position, {fill_width, rect.size.y}, color_fill);
        }
    }
};

} // namespace nova::ui

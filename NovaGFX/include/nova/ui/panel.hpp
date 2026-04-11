#pragma once
#include "nova/ui/control.hpp"

namespace nova::ui {

class Panel : public Control {
public:
    Color background_color{0.15f, 0.15f, 0.15f, 0.8f};
    Color border_color{0.3f, 0.3f, 0.3f, 1.0f};
    f32 border_thickness{1.0f};

    Panel() {
        size = {200, 200};
    }

    void draw(Renderer2D& renderer) override {
        Rect2f rect = get_global_rect();
        renderer.draw_rect(rect.position, rect.size, background_color);
        if (border_thickness > 0) {
            renderer.draw_rect_outline(rect.position, rect.size, border_color, border_thickness);
        }
    }
};

} // namespace nova::ui

#pragma once
#include "nova/scene/node2d.hpp"
#include "nova/math/rect2.hpp"
#include "nova/input/input.hpp"

namespace nova::ui {

class Control : public Node2D {
public:
    Vector2f size{0, 0};
    bool is_hovered{false};
    bool is_focused{false};

    Control() = default;
    virtual ~Control() = default;

    Rect2f get_global_rect() const;

    // Returns true if input was consumed
    virtual bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released);

    // Updates the control hierarchy input
    // Call this instead of update_tree to handle UI specific logic properly
    bool update_ui_tree(f32 delta, Vector2f mouse_pos, bool mouse_pressed, bool mouse_released);
};

} // namespace nova::ui

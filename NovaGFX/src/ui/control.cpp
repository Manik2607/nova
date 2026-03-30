#include "nova/ui/control.hpp"

namespace nova::ui {

Rect2f Control::get_global_rect() const {
    Transform2D global = get_global_transform();
    // Assuming UI has little rotation/scale natively or if it does, this approximation holds
    // For exact OBB math, Rect2 limits would be needed, but AABB is fine here
    return Rect2f(global.origin, size);
}

bool Control::handle_input(Vector2f mouse_pos, bool /*mouse_pressed*/, bool /*mouse_released*/) {
    is_hovered = get_global_rect().contains(mouse_pos);
    return false; // By default don't consume
}

bool Control::update_ui_tree(f32 delta, Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) {
    update(delta); // Node2D user update

    bool consumed = false;
    
    // Process children in reverse order (top-most elements first)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        Control* child = dynamic_cast<Control*>(it->get());
        if (child) {
            if (child->update_ui_tree(delta, mouse_pos, mouse_pressed, mouse_released) && !consumed) {
                consumed = true;
            }
        } else {
            (*it)->update_tree(delta); // Normal node update fallback
        }
    }

    if (!consumed) {
        consumed = handle_input(mouse_pos, mouse_pressed, mouse_released);
    } else {
        // If input was consumed by child, we are not hovered/focused for this mouse event
        is_hovered = false;
    }

    return consumed;
}

} // namespace nova::ui

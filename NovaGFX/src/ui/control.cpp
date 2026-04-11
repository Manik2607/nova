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

void Control::layout_children() {
    // Base implementation does nothing or simple anchoring
    for (auto& child_raw : m_children) {
        Control* child = dynamic_cast<Control*>(child_raw.get());
        if (!child || child->anchor == Anchor::NONE) continue;
        
        switch (child->anchor) {
            case Anchor::TOP_LEFT:      child->position = {child->margins.x, child->margins.y}; break;
            case Anchor::TOP_CENTER:    child->position = {size.x * 0.5f - child->size.x * 0.5f, child->margins.y}; break;
            case Anchor::TOP_RIGHT:     child->position = {size.x - child->size.x - child->margins.z, child->margins.y}; break;
            case Anchor::CENTER_LEFT:   child->position = {child->margins.x, size.y * 0.5f - child->size.y * 0.5f}; break;
            case Anchor::CENTER:        child->position = {size.x * 0.5f - child->size.x * 0.5f, size.y * 0.5f - child->size.y * 0.5f}; break;
            case Anchor::CENTER_RIGHT:  child->position = {size.x - child->size.x - child->margins.z, size.y * 0.5f - child->size.y * 0.5f}; break;
            case Anchor::BOTTOM_LEFT:   child->position = {child->margins.x, size.y - child->size.y - child->margins.w}; break;
            case Anchor::BOTTOM_CENTER: child->position = {size.x * 0.5f - child->size.x * 0.5f, size.y - child->size.y - child->margins.w}; break;
            case Anchor::BOTTOM_RIGHT:  child->position = {size.x - child->size.x - child->margins.z, size.y - child->size.y - child->margins.w}; break;
            case Anchor::FILL:
                child->position = {child->margins.x, child->margins.y};
                child->size = {size.x - child->margins.x - child->margins.z, size.y - child->margins.y - child->margins.w};
                break;
            default: break;
        }
    }
}

bool Control::update_ui_tree(f32 delta, Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) {
    if (!visible) return false;
    layout_children();
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

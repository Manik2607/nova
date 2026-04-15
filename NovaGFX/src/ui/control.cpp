#include "nova/ui/control.hpp"
#include <algorithm>

namespace nova::ui {

Rect2f Control::get_global_rect() const {
    Transform2D global = get_global_transform();
    return Rect2f(global.origin, size);
}

bool Control::handle_input(Vector2f mouse_pos, bool /*mouse_pressed*/, bool /*mouse_released*/) {
    is_hovered = get_global_rect().contains(mouse_pos);
    return false;
}

void Control::set_width_propagate(f32 w) {
    size.x = w;
    for (auto& cr : m_children) {
        Control* child = dynamic_cast<Control*>(cr.get());
        if (child && child->anchor == Anchor::NONE) {
            child->set_width_propagate(w);
        }
    }
}

void Control::layout_children() {
    // ---------------------------------------------------------------
    // Pass 1: anchored children — we control their position/size
    // ---------------------------------------------------------------
    for (auto& child_raw : m_children) {
        Control* child = dynamic_cast<Control*>(child_raw.get());
        if (!child || !child->visible || child->anchor == Anchor::NONE) continue;

        switch (child->anchor) {
            case Anchor::TOP_LEFT:
                child->position = {child->margins.x, child->margins.y};
                break;
            case Anchor::TOP_CENTER:
                child->position = {size.x * 0.5f - child->size.x * 0.5f, child->margins.y};
                break;
            case Anchor::TOP_RIGHT:
                child->position = {size.x - child->size.x - child->margins.z, child->margins.y};
                break;
            case Anchor::CENTER_LEFT:
                child->position = {child->margins.x, size.y * 0.5f - child->size.y * 0.5f};
                break;
            case Anchor::CENTER:
                child->position = {size.x * 0.5f - child->size.x * 0.5f,
                                   size.y * 0.5f - child->size.y * 0.5f};
                break;
            case Anchor::CENTER_RIGHT:
                child->position = {size.x - child->size.x - child->margins.z,
                                   size.y * 0.5f - child->size.y * 0.5f};
                break;
            case Anchor::BOTTOM_LEFT:
                child->position = {child->margins.x, size.y - child->size.y - child->margins.w};
                break;
            case Anchor::BOTTOM_CENTER:
                child->position = {size.x * 0.5f - child->size.x * 0.5f,
                                   size.y - child->size.y - child->margins.w};
                break;
            case Anchor::BOTTOM_RIGHT:
                child->position = {size.x - child->size.x - child->margins.z,
                                   size.y - child->size.y - child->margins.w};
                break;
            case Anchor::FILL:
                child->position = {child->margins.x, child->margins.y};
                child->size     = {size.x - child->margins.x - child->margins.z,
                                   size.y - child->margins.y - child->margins.w};
                break;
            default: break;
        }
        // Recurse so anchored child arranges its own children
        child->layout_children();
    }

    // ---------------------------------------------------------------
    // Pass 2: non-anchored children — they position themselves,
    //         we just call their layout and track bounds for auto-sizing.
    // ---------------------------------------------------------------
    Vector2f bounds{0.0f, 0.0f};
    for (auto& child_raw : m_children) {
        Control* child = dynamic_cast<Control*>(child_raw.get());
        if (!child || !child->visible || child->anchor != Anchor::NONE) continue;

        child->layout_children();

        bounds.x = std::max(bounds.x, child->position.x + child->size.x);
        bounds.y = std::max(bounds.y, child->position.y + child->size.y);
    }

    // Auto-size self to encompass non-anchored children.
    // This is what makes content wrappers (ScrollView) know their height.
    if (!expand) {
        if (bounds.x > size.x) size.x = bounds.x;
        if (bounds.y > size.y) size.y = bounds.y;
    }
}

bool Control::update_ui_tree(f32 delta, Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) {
    if (!visible) return false;

    // Layout before update so sizes/positions are correct for this frame
    layout_children();
    update(delta);

    // Set is_hovered from raw mouse position BEFORE child processing,
    // so ScrollViews etc. always know if mouse is over them.
    is_hovered = get_global_rect().contains(mouse_pos);

    bool consumed = false;

    // Children in reverse draw order (topmost first for input)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        Control* child = dynamic_cast<Control*>(it->get());
        if (child) {
            if (child->update_ui_tree(delta, mouse_pos, mouse_pressed, mouse_released) && !consumed) {
                consumed = true;
            }
        } else {
            (*it)->update_tree(delta);
        }
    }

    if (!consumed) {
        consumed = handle_input(mouse_pos, mouse_pressed, mouse_released);
    }

    return consumed;
}

} // namespace nova::ui

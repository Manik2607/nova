#pragma once
#include "nova/ui/control.hpp"

namespace nova::ui {

class BoxContainer : public Control {
public:
    enum class Orientation { HORIZONTAL, VERTICAL };
    
    Orientation orientation{Orientation::HORIZONTAL};
    f32 spacing{5.0f};

    void layout_children() override {
        Vector2f current_pos = {padding.x, padding.y};
        for (auto& child_raw : m_children) {
            Control* child = dynamic_cast<Control*>(child_raw.get());
            if (!child) continue;

            if (orientation == Orientation::HORIZONTAL) {
                if (child->visible) {
                    if (child->expand) child->size.y = size.y - padding.y - padding.w;
                    child->position = current_pos;
                    current_pos.x += child->size.x + spacing;
                }
            } else {
                if (child->visible) {
                    if (child->expand) child->size.x = size.x - padding.x - padding.z;
                    child->position = current_pos;
                    current_pos.y += child->size.y + spacing;
                }
            }
        }
        
        // Auto-size based on children
        if (orientation == Orientation::HORIZONTAL) {
            size.x = current_pos.x - spacing + padding.z;
            size.y = 0;
            for (auto& child_raw : m_children) {
                if (Control* child = dynamic_cast<Control*>(child_raw.get())) {
                    size.y = std::max(size.y, child->size.y + padding.y + padding.w);
                }
            }
        } else {
            size.y = current_pos.y - spacing + padding.w;
            size.x = 0;
            for (auto& child_raw : m_children) {
                if (Control* child = dynamic_cast<Control*>(child_raw.get())) {
                    size.x = std::max(size.x, child->size.x + padding.x + padding.z);
                }
            }
        }
    }
};

class HBoxContainer : public BoxContainer {
public:
    HBoxContainer() { orientation = Orientation::HORIZONTAL; }
};

class VBoxContainer : public BoxContainer {
public:
    VBoxContainer() { orientation = Orientation::VERTICAL; }
};

} // namespace nova::ui

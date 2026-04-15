#pragma once
#include "nova/ui/control.hpp"
#include <algorithm>
#include <vector>

namespace nova::ui {

class BoxContainer : public Control {
public:
    enum class Orientation { HORIZONTAL, VERTICAL };

    Orientation orientation{Orientation::HORIZONTAL};
    f32 spacing{5.0f};

    void layout_children() override {
        // Collect visible children
        std::vector<Control*> children;
        for (auto& cr : m_children) {
            Control* c = dynamic_cast<Control*>(cr.get());
            if (c && c->visible) children.push_back(c);
        }
        if (children.empty()) return;

        // Count expand children
        int expand_count = 0;
        for (Control* c : children) {
            if (c->expand) expand_count++;
        }

        if (orientation == Orientation::HORIZONTAL) {
            // Fixed-width children
            f32 fixed_w = 0.0f;
            for (Control* c : children) {
                if (!c->expand) fixed_w += c->size.x;
            }
            f32 total_spacing = spacing * (f32)(children.size() > 0 ? (int)children.size() - 1 : 0);
            f32 avail         = size.x - padding.x - padding.z - fixed_w - total_spacing;
            f32 expand_w      = (expand_count > 0 && size.x > 0) ? std::max(0.0f, avail / expand_count) : 0.0f;

            f32 cx = padding.x;
            for (Control* c : children) {
                if (c->expand) c->size.x = expand_w;
                // Only force cross-axis if we know our height
                if (size.y > 0) c->size.y = size.y - padding.y - padding.w;
                c->position = {cx, padding.y};
                cx += c->size.x + spacing;
            }

            // Auto-size if container is not expand
            if (!expand) {
                f32 new_w = cx - spacing + padding.z;
                size.x = std::max(size.x, new_w);
                f32 max_h = 0.0f;
                for (Control* c : children) max_h = std::max(max_h, c->size.y);
                size.y = max_h + padding.y + padding.w;
            }
        } else {
            // VERTICAL
            f32 fixed_h = 0.0f;
            for (Control* c : children) {
                if (!c->expand) fixed_h += c->size.y;
            }
            f32 total_spacing = spacing * (f32)(children.size() > 0 ? (int)children.size() - 1 : 0);
            f32 avail         = size.y - padding.y - padding.w - fixed_h - total_spacing;
            f32 expand_h      = (expand_count > 0 && size.y > 0) ? std::max(0.0f, avail / expand_count) : 0.0f;

            f32 cy = padding.y;
            for (Control* c : children) {
                if (c->expand) c->size.y = expand_h;
                // Only force cross-axis width if we know our own width
                if (size.x > 0) c->size.x = size.x - padding.x - padding.z;
                c->position = {padding.x, cy};
                cy += c->size.y + spacing;
            }

            // Auto-size if container is not expand
            if (!expand) {
                f32 new_h = cy - spacing + padding.w;
                size.y = std::max(size.y, new_h);
                f32 max_w = 0.0f;
                for (Control* c : children) max_w = std::max(max_w, c->size.x);
                if (size.x == 0) size.x = max_w + padding.x + padding.z;
            }
        }

        // Recurse into all children
        for (Control* c : children) {
            c->layout_children();
        }
    }

    // Override: don't recurse into children — layout_children handles
    // assigning child widths based on orientation, expand flags, etc.
    void set_width_propagate(f32 w) override {
        size.x = w;
        // Stop here. Our layout_children will set children's widths correctly.
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

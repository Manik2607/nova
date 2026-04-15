#pragma once
#include "nova/ui/control.hpp"
#include <algorithm>
#include <vector>

namespace nova::ui {

class GridContainer : public Control {
public:
    int columns{2};
    f32 h_spacing{5.0f};
    f32 v_spacing{5.0f};
    f32 spacing{5.0f}; // convenience — synced in layout

    void layout_children() override {
        h_spacing = v_spacing = spacing;
        if (m_children.empty()) return;

        // Collect visible children
        std::vector<Control*> children;
        for (auto& cr : m_children) {
            Control* c = dynamic_cast<Control*>(cr.get());
            if (c && c->visible) children.push_back(c);
        }
        if (children.empty()) return;

        // Available inner width (respect padding)
        f32 avail_w = size.x - padding.x - padding.z;
        // Compute uniform column width
        int cols = std::max(1, columns);
        f32 col_w = (avail_w - (cols - 1) * h_spacing) / cols;

        // First pass: set widths and compute per-row heights
        std::vector<f32> row_heights;
        int i = 0;
        for (Control* c : children) {
            int col = i % cols;
            int row = i / cols;

            if (avail_w > 0) c->size.x = std::max(0.0f, col_w);

            if (row >= (int)row_heights.size()) row_heights.push_back(0.0f);
            row_heights[row] = std::max(row_heights[row], c->size.y);

            i++;
        }

        // Second pass: position children
        i = 0;
        f32 cy = padding.y;
        for (int row = 0; row < (int)row_heights.size(); ++row) {
            f32 cx = padding.x;
            for (int col = 0; col < cols && i < (int)children.size(); ++col, ++i) {
                children[i]->position = {cx, cy};
                children[i]->layout_children();
                cx += col_w + h_spacing;
            }
            cy += row_heights[row] + v_spacing;
        }

        // Auto-size only if we are not set to expand (preserve parent-given size)
        if (!expand) {
            size.x = padding.x + padding.z + cols * col_w + (cols - 1) * h_spacing;
            size.y = padding.y + padding.w;
            for (f32 h : row_heights) size.y += h + v_spacing;
            if (!row_heights.empty()) size.y -= v_spacing;
        }
    }
};

} // namespace nova::ui

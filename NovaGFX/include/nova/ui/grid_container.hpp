#pragma once
#include "nova/ui/control.hpp"

namespace nova::ui {

class GridContainer : public Control {
public:
    int columns{2};
    f32 h_spacing{5.0f};
    f32 v_spacing{5.0f};
    f32 spacing{5.0f}; // convenience for both

    void layout_children() override {
        h_spacing = v_spacing = spacing; // Sync with convenience field
        if (m_children.empty()) return;

        std::vector<f32> row_heights;
        std::vector<f32> col_widths(columns, 0.0f);

        // First pass: calculate dimensions
        f32 available_width = size.x - padding.x - padding.z;
        f32 adaptive_col_width = (available_width - (columns - 1) * h_spacing) / columns;

        int i = 0;
        for (auto& child_raw : m_children) {
            Control* child = dynamic_cast<Control*>(child_raw.get());
            if (!child || !child->visible) continue;

            int col = i % columns;
            int row = i / columns;
            
            if (row >= (int)row_heights.size()) row_heights.push_back(0);
            
            // If the container has a set width (e.g. from expand or manual), use adaptive width
            if (available_width > 0) {
                child->size.x = adaptive_col_width;
            }
            
            col_widths[col] = std::max(col_widths[col], child->size.x);
            row_heights[row] = std::max(row_heights[row], child->size.y);
            i++;
        }

        // Second pass: position children
        i = 0;
        Vector2f current_pos = {padding.x, padding.y};
        for (auto& child_raw : m_children) {
            Control* child = dynamic_cast<Control*>(child_raw.get());
            if (!child || !child->visible) continue;

            int col = i % columns;
            int row = i / columns;

            child->position = current_pos;

            if (col < columns - 1) {
                current_pos.x += col_widths[col] + h_spacing;
            } else {
                current_pos.x = padding.x;
                current_pos.y += row_heights[row] + v_spacing;
            }
            i++;
        }
        
        // Auto-size
        size.x = padding.x + padding.z;
        for (f32 w : col_widths) size.x += w + h_spacing;
        if (!col_widths.empty()) size.x -= h_spacing;
        
        size.y = padding.y + padding.w;
        for (f32 h : row_heights) size.y += h + v_spacing;
        if (!row_heights.empty()) size.y -= v_spacing;
    }
};

} // namespace nova::ui

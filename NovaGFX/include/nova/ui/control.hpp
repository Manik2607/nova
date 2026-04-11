#pragma once
#include "nova/scene/node2d.hpp"
#include "nova/math/rect2.hpp"
#include "nova/input/input.hpp"

#include "nova/math/vector4.hpp"

namespace nova::ui {

enum class Anchor {
    TOP_LEFT, TOP_CENTER, TOP_RIGHT,
    CENTER_LEFT, CENTER, CENTER_RIGHT,
    BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT,
    FILL, // Fills the parent container
    NONE
};

class Control : public Node2D {
public:
    Vector2f size{0, 0};
    Vector2f min_size{0, 0};
    Vector2f max_size{10000, 10000};
    
    // x:left, y:top, z:right, w:bottom
    Vector4f margins{0, 0, 0, 0};
    Vector4f padding{0, 0, 0, 0};
    
    Anchor anchor{Anchor::NONE};
    
    bool is_hovered{false};
    bool is_focused{false};
    bool expand{false}; // Hints to containers to expand this node

    Control() = default;
    virtual ~Control() = default;

    Rect2f get_global_rect() const;

    // Optional override for containers to position children
    virtual void layout_children();

    // Returns true if input was consumed
    virtual bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released);

    // Updates the control hierarchy input
    // Call this instead of update_tree to handle UI specific logic properly
    bool update_ui_tree(f32 delta, Vector2f mouse_pos, bool mouse_pressed, bool mouse_released);
};

} // namespace nova::ui

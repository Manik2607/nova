#pragma once
#include "nova/ui/control.hpp"
#include "nova/input/input.hpp"
#include <cmath>
#include <algorithm>

namespace nova::ui {

class ScrollView : public Control {
public:
    Vector2f scroll_pos{0, 0};
    bool scroll_v{true};
    bool scroll_h{false};
    Color background_color{0.08f, 0.08f, 0.12f, 1.0f};
    f32 scroll_speed{50.0f};

    // Children should be added to `content`, not directly to ScrollView.
    // content->position is set to -scroll_pos so global transforms are correct
    // for hit-testing (buttons inside work at the scroll offset).
    Control* content{nullptr};

    ScrollView() {
        size = {200, 200};
        content = add_child<Control>();
        content->size = {0, 0};
    }

    void layout_children() override {
        if (!content) return;

        const f32 bar_w = scroll_v ? 8.0f : 0.0f;
        const f32 inner_w = size.x - bar_w;

        // Propagate width down so inner VBoxes know how wide to be
        content->set_width_propagate(inner_w);

        // Reset content height so it re-grows from scratch this frame
        content->size = {inner_w, 0.0f};

        // Layout content (Pass 2 inside Control::layout_children auto-sizes
        // content to encompass its non-anchored VBox child)
        content->layout_children();

        // Apply scroll offset — critical: this is what makes child
        // global rects correct for hit-testing
        content->position = {0.0f, -scroll_pos.y};
        if (scroll_h) content->position.x = -scroll_pos.x;

        clamp_scroll();
    }

    void draw_tree(Renderer2D& renderer) override {
        if (!visible) return;

        Rect2f rect = get_global_rect();

        // Background (transparent or solid)
        if (background_color.a > 0.0f) {
            renderer.draw_rect(rect.position, rect.size, background_color);
        }

        // Layout must be up-to-date before drawing
        layout_children();

        // Clip all children to our rect
        renderer.set_clip_rect(rect);

        draw(renderer); // our own draw (nothing for base ScrollView)
        for (auto& child : m_children) {
            child->draw_tree(renderer);
        }

        renderer.clear_clip_rect();

        // Draw scrollbar OUTSIDE clip so it's always fully visible
        if (scroll_v) draw_scrollbar_v(renderer);
    }

    bool handle_input(Vector2f mouse_pos, bool mouse_pressed, bool mouse_released) override {
        Rect2f self_rect = get_global_rect();
        is_hovered = self_rect.contains(mouse_pos);

        // ---- Vertical scrollbar thumb interaction ----
        if (scroll_v && content && content->size.y > size.y) {
            Rect2f thumb = get_thumb_rect_v();
            m_thumb_hovered_v = thumb.contains(mouse_pos);

            if (m_thumb_hovered_v && mouse_pressed && !m_dragging_v) {
                m_dragging_v        = true;
                m_drag_start_mouse  = mouse_pos.y;
                m_drag_start_scroll = scroll_pos.y;
                return true;
            }
        }

        if (m_dragging_v) {
            if (Input::is_mouse_down(MouseButton::LEFT)) {
                f32 max_v     = std::max(0.0f, content->size.y - size.y);
                f32 thumb_h   = get_thumb_height_v();
                f32 usable    = size.y - thumb_h;
                if (usable > 0.0f) {
                    f32 dy = mouse_pos.y - m_drag_start_mouse;
                    scroll_pos.y = m_drag_start_scroll + dy * (max_v / usable);
                    clamp_scroll();
                }
                return true;
            } else {
                m_dragging_v = false;
            }
        }

        return false;
    }

    void update(f32 /*delta*/) override {
        if (!is_hovered && !m_dragging_v) return;

        Vector2f scroll_delta = Input::get_scroll_delta();

        if (scroll_v && std::abs(scroll_delta.y) > 0.01f) {
            scroll_pos.y -= scroll_delta.y * scroll_speed;
            clamp_scroll();
        }
        if (scroll_h && std::abs(scroll_delta.x) > 0.01f) {
            scroll_pos.x -= scroll_delta.x * scroll_speed;
            clamp_scroll();
        }
    }

    void clamp_scroll() {
        if (content) {
            f32 max_v = std::max(0.0f, content->size.y - size.y);
            f32 max_h = std::max(0.0f, content->size.x - size.x);
            scroll_pos.y = std::clamp(scroll_pos.y, 0.0f, max_v);
            scroll_pos.x = std::clamp(scroll_pos.x, 0.0f, max_h);
        }
    }

    template<typename T, typename... Args>
    T* add_to_content(Args&&... args) {
        return content->add_child<T>(std::forward<Args>(args)...);
    }

private:
    // ---- scrollbar state ----
    bool  m_thumb_hovered_v{false};
    bool  m_dragging_v{false};
    f32   m_drag_start_mouse{0.0f};
    f32   m_drag_start_scroll{0.0f};

    static constexpr f32 BAR_W = 8.0f;

    f32 get_thumb_height_v() const {
        if (!content || content->size.y <= size.y) return size.y;
        f32 ratio = std::clamp(size.y / content->size.y, 0.05f, 1.0f);
        return std::max(size.y * ratio, 24.0f);
    }

    Rect2f get_thumb_rect_v() const {
        Rect2f rect = get_global_rect();
        f32 max_v   = std::max(0.0f, content->size.y - size.y);
        f32 thumb_h = get_thumb_height_v();
        f32 usable  = size.y - thumb_h;
        f32 t       = (max_v > 0.0f) ? (scroll_pos.y / max_v) : 0.0f;
        f32 thumb_y = t * usable;
        return Rect2f({rect.position.x + rect.size.x - BAR_W, rect.position.y + thumb_y},
                      {BAR_W, thumb_h});
    }

    void draw_scrollbar_v(Renderer2D& renderer) {
        if (!content || content->size.y <= size.y) return;

        Rect2f rect  = get_global_rect();
        f32 track_x  = rect.position.x + rect.size.x - BAR_W;
        f32 track_y  = rect.position.y;
        f32 track_h  = rect.size.y;

        // Track
        renderer.draw_rect({track_x, track_y}, {BAR_W, track_h},
                           {0.08f, 0.08f, 0.14f, 0.95f});

        // Thumb
        Rect2f thumb = get_thumb_rect_v();
        Color thumb_col = m_thumb_hovered_v || m_dragging_v
                        ? Color{0.55f, 0.70f, 1.0f, 1.0f}
                        : Color{0.35f, 0.48f, 0.75f, 0.90f};
        renderer.draw_rect(thumb.position + Vector2f{1.0f, 0.0f},
                           {thumb.size.x - 2.0f, thumb.size.y},
                           thumb_col);
    }
};

} // namespace nova::ui

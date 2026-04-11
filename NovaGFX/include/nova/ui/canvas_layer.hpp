#pragma once
#include "nova/ui/control.hpp"

namespace nova::ui {

class CanvasLayer : public Control {
public:
    void draw_tree(Renderer2D& renderer) override {
        // We don't have a way to easily 'save' camera state in Renderer2D right now 
        // without adding more state.
        // For now, let's just assume this is a top-level node and it sets the UI projection.
        // In a real engine, we'd use a stack or separate render pass.
        
        // Let's assume viewport size is stored in the renderer (which it is now)
        // We'll set camera to center of viewport, 1.0 zoom.
        // Vector2f viewport = renderer.get_viewport_size(); // I should add a getter
        
        // Actually, for now, let's just make it a marker and let the user call init_camera 
        // as they do in the example, OR I add the getter.
        
        // I'll add the getter to Renderer2D.
        
        // renderer.init_camera(m_last_viewport * 0.5f, 1.0f, m_last_viewport);
        
        draw(renderer);
        for (auto& child : m_children) {
            child->draw_tree(renderer);
        }
    }
};

} // namespace nova::ui

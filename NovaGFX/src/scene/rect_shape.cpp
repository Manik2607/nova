#include "nova/scene/rect_shape.hpp"
#include <cmath>

namespace nova {

void RectShape::draw(Renderer2D& renderer) {
    Transform2D global_transform = get_global_transform();
    
    Vector2f global_pos = global_transform.xform(Vector2f(0, 0));
    Vector2f global_scale = {global_transform.x_axis.length(), global_transform.y_axis.length()};
    f32 global_rot = std::atan2(global_transform.x_axis.y, global_transform.x_axis.x);

    if (filled) {
        renderer.draw_rect(global_pos, size * global_scale, color, global_rot);
    } else {
        renderer.draw_rect_outline(global_pos, size * global_scale, color, 1.0f, global_rot);
    }
}

} // namespace nova

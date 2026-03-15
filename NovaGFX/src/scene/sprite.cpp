#include "nova/scene/sprite.hpp"
#include <cmath>

namespace nova {

void Sprite::draw(Renderer2D& renderer) {
    if (texture) {
        Transform2D global_transform = get_global_transform();
        
        Vector2f global_pos = global_transform.xform(Vector2f(0, 0));
        Vector2f global_scale = {global_transform.x_axis.length(), global_transform.y_axis.length()};
        f32 global_rot = std::atan2(global_transform.x_axis.y, global_transform.x_axis.x);

        renderer.draw_sprite(*texture, global_pos, size * global_scale, tint, global_rot, origin);
    }
}

} // namespace nova

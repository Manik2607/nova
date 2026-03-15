#include "nova/scene/polygon2d.hpp"

namespace nova {

void Polygon2D::draw(Renderer2D& renderer) {
    if (points.empty()) return;

    Transform2D global_transform = get_global_transform();
    
    std::vector<Vector2f> global_points;
    global_points.reserve(points.size());
    for (const auto& p : points) {
        global_points.push_back(global_transform.xform(p));
    }

    if (filled) {
        renderer.draw_polygon(global_points, color);
    } else {
        renderer.draw_polygon_outline(global_points, color, 1.0f);
    }
}

} // namespace nova

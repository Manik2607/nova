#pragma once
#include "node2d.hpp"
#include "nova/math/color.hpp"
#include <vector>

namespace nova {

class Renderer2D;

class Polygon2D : public Node2D {
public:
    std::vector<Vector2f> points;
    Color color{Color::WHITE()};
    bool filled{true};

    Polygon2D() = default;

    void draw(Renderer2D& renderer) override;
};

} // namespace nova

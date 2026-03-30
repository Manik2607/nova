#pragma once
#include "node2d.hpp"
#include "nova/math/color.hpp"

namespace nova {

class Renderer2D;

class RectShape : public Node2D {
public:
    Vector2f size{100, 100};
    Color color{Color::WHITE()};
    bool filled{true};

    RectShape() = default;

    void draw(Renderer2D& renderer) override;
};

} // namespace nova

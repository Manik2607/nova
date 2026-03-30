#pragma once
#include "node2d.hpp"
#include "nova/assets/texture2d.hpp"

namespace nova {

class Renderer2D;

class Sprite : public Node2D {
public:
    const Texture2D* texture{nullptr};
    Color tint{Color::WHITE()};
    Vector2f size{0, 0};
    Vector2f origin{0.5f, 0.5f};

    Sprite() = default;

    void draw(Renderer2D& renderer) override;
};

} // namespace nova

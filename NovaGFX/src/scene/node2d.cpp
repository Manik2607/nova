#include "nova/scene/node2d.hpp"

namespace nova {

Transform2D Node2D::get_local_transform() const {
    Transform2D t = Transform2D::from_position(position);
    if (rotation != 0.0f) t = t.rotated(rotation);
    if (scale != Vector2f::ONE()) t = t.scaled(scale);
    return t;
}

Transform2D Node2D::get_global_transform() const {
    if (m_parent) {
        return m_parent->get_global_transform() * get_local_transform();
    }
    return get_local_transform();
}

Node2D* Node2D::add_child(std::unique_ptr<Node2D> child) {
    if (child) {
        child->m_parent = this;
        Node2D* ptr = child.get();
        m_children.push_back(std::move(child));
        return ptr;
    }
    return nullptr;
}

void Node2D::update(f32 delta) {
    (void)delta;
}

void Node2D::draw(Renderer2D& renderer) {
    (void)renderer;
}

void Node2D::update_tree(f32 delta) {
    if (!visible) return;
    update(delta);
    for (auto& child : m_children) {
        child->update_tree(delta);
    }
}

void Node2D::draw_tree(Renderer2D& renderer) {
    if (!visible) return;
    draw(renderer);
    for (auto& child : m_children) {
        child->draw_tree(renderer);
    }
}

} // namespace nova

#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/transform2d.hpp"
#include "nova/renderer/renderer2d.hpp"
#include <vector>
#include <memory>

namespace nova {

class Node2D {
public:
    Vector2f position{0, 0};
    f32 rotation{0};
    Vector2f scale{1, 1};
    bool visible{true};

    Node2D() = default;
    virtual ~Node2D() = default;

    void set_visible(bool v) { visible = v; }
    bool is_visible() const { return visible; }

    Node2D(const Node2D&) = delete;
    Node2D& operator=(const Node2D&) = delete;
    Node2D(Node2D&&) = default;
    Node2D& operator=(Node2D&&) = default;

    Transform2D get_local_transform() const;
    Transform2D get_global_transform() const;

    Node2D* add_child(std::unique_ptr<Node2D> child);

    template<typename T, typename... Args>
    T* add_child(Args&&... args) {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = child.get();
        add_child(std::move(child));
        return ptr;
    }

    virtual void update(f32 delta);
    virtual void draw(Renderer2D& renderer);

    virtual void update_tree(f32 delta);
    virtual void draw_tree(Renderer2D& renderer);

    Node2D* get_parent() const { return m_parent; }

protected:
    Node2D* m_parent{nullptr};
    std::vector<std::unique_ptr<Node2D>> m_children;
};

} // namespace nova

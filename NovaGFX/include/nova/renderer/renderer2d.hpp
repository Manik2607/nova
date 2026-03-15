#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/color.hpp"
#include "nova/math/rect2.hpp"
#include "nova/assets/texture2d.hpp"
#include "nova/assets/shader.hpp"
#include <vector>
#include <span>
#include <string_view>
#include <memory>

namespace nova {

struct FrameStats {
    u32 draw_calls{0};
    u32 vertex_count{0};
    u32 quad_count{0};
};

class Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    void init_camera(Vector2f position, f32 zoom, Vector2f viewport_size);

    void begin();
    void end();

    void draw_rect(Vector2f position, Vector2f size, Color color,
                   f32 rotation = 0.0f, Vector2f origin = Vector2f::ZERO());

    void draw_rect_outline(Vector2f position, Vector2f size, Color color,
                           f32 thickness = 1.0f, f32 rotation = 0.0f);

    void draw_sprite(const Texture2D& tex, Vector2f position, Vector2f size,
                     Color tint = Color::WHITE(), f32 rotation = 0.0f,
                     Vector2f origin = {0.5f, 0.5f});

    void draw_sprite_region(const Texture2D& tex, Rect2f src_rect,
                            Vector2f position, Vector2f size,
                            Color tint = Color::WHITE(), f32 rotation = 0.0f);

    void draw_circle(Vector2f center, f32 radius, Color color,
                     i32 segments = 32);

    void draw_circle_outline(Vector2f center, f32 radius, Color color,
                             f32 thickness = 1.0f, i32 segments = 32);

    void draw_line(Vector2f a, Vector2f b, Color color, f32 thickness = 1.0f);

    void draw_polygon(std::span<const Vector2f> points, Color color);

    void draw_polygon_outline(std::span<const Vector2f> points,
                              Color color, f32 thickness = 1.0f);

    void draw_text(std::string_view text, Vector2f position, Color color,
                   f32 size); // stub

    const FrameStats& get_stats() const { return m_stats; }

private:
    void flush();
    void push_quad(const Vector2f* positions, const Vector2f* uvs, Color color, f32 tex_index);
    void push_triangle(const Vector2f* positions, const Vector2f* uvs, Color color, f32 tex_index);
    f32 get_texture_index(const Texture2D* tex);

    #pragma pack(push, 1)
    struct Vertex2D {
        Vector2f position;
        Vector2f uv;
        Color color;
        f32 tex_index;
    };
    #pragma pack(pop)

    static constexpr usize MAX_QUADS = 16384;
    static constexpr usize MAX_VERTICES = MAX_QUADS * 4;
    static constexpr usize MAX_INDICES = MAX_QUADS * 6;
    static constexpr usize MAX_TEXTURE_SLOTS = 16;

    u32 m_vao{0};
    u32 m_vbo{0};
    u32 m_ebo{0};

    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Texture2D> m_white_texture;

    std::vector<Vertex2D> m_vertices;
    u32 m_vertex_count{0};
    u32 m_index_count{0};

    const Texture2D* m_textures[MAX_TEXTURE_SLOTS];
    u32 m_texture_count{1}; // 0 is white texture

    FrameStats m_stats;
    FrameStats m_stats_current;
};

} // namespace nova

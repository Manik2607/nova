#include "nova/renderer/renderer2d.hpp"
#include <glad/glad.h>
#include <cmath>

namespace nova {

static const char* VERTEX_SHADER_SRC = R"(
#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_TexIndex;

out vec2 v_TexCoord;
out vec4 v_Color;
out float v_TexIndex;

uniform mat4 u_Projection;

void main() {
    v_TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    v_Color = a_Color;
    v_TexIndex = a_TexIndex;
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
}
)";

static const char* FRAGMENT_SHADER_SRC = R"(
#version 330 core
out vec4 FragColor;

in vec2 v_TexCoord;
in vec4 v_Color;
in float v_TexIndex;

uniform sampler2D u_Textures[16];

void main() {
    int index = int(v_TexIndex + 0.5);
    vec4 texColor = vec4(1.0);

    switch(index) {
        case 0: texColor = texture(u_Textures[0], v_TexCoord); break;
        case 1: texColor = texture(u_Textures[1], v_TexCoord); break;
        case 2: texColor = texture(u_Textures[2], v_TexCoord); break;
        case 3: texColor = texture(u_Textures[3], v_TexCoord); break;
        case 4: texColor = texture(u_Textures[4], v_TexCoord); break;
        case 5: texColor = texture(u_Textures[5], v_TexCoord); break;
        case 6: texColor = texture(u_Textures[6], v_TexCoord); break;
        case 7: texColor = texture(u_Textures[7], v_TexCoord); break;
        case 8: texColor = texture(u_Textures[8], v_TexCoord); break;
        case 9: texColor = texture(u_Textures[9], v_TexCoord); break;
        case 10: texColor = texture(u_Textures[10], v_TexCoord); break;
        case 11: texColor = texture(u_Textures[11], v_TexCoord); break;
        case 12: texColor = texture(u_Textures[12], v_TexCoord); break;
        case 13: texColor = texture(u_Textures[13], v_TexCoord); break;
        case 14: texColor = texture(u_Textures[14], v_TexCoord); break;
        case 15: texColor = texture(u_Textures[15], v_TexCoord); break;
    }

    FragColor = texColor * v_Color;
}
)";

Renderer2D::Renderer2D() {
    m_vertices.resize(MAX_VERTICES);
    for (usize i = 0; i < MAX_TEXTURE_SLOTS; ++i) m_textures[i] = nullptr;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex2D), nullptr, GL_DYNAMIC_DRAW);

    std::vector<u32> indices(MAX_INDICES);
    u32 offset = 0;
    for (usize i = 0; i < MAX_INDICES; i += 6) {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;

        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(u32), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, position));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, uv));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, color));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (const void*)offsetof(Vertex2D, tex_index));

    glBindVertexArray(0);

    m_shader = std::make_unique<Shader>(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);

    m_shader->bind();
    for (usize i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        std::string name = "u_Textures[" + std::to_string(i) + "]";
        m_shader->set_int(name, static_cast<nova::i32>(i));
    }
    m_shader->unbind();

    u32 white_pixel = 0xFFFFFFFF;
    m_white_texture = std::make_unique<Texture2D>((const u8*)&white_pixel, 1, 1, 4, Texture2D::FilterMode::NEAREST);
    m_textures[0] = m_white_texture.get();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer2D::~Renderer2D() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void Renderer2D::init_camera(Vector2f position, f32 zoom, Vector2f viewport_size) {
    if (zoom <= 0.0001f) zoom = 0.0001f;
    Vector2f size = viewport_size / zoom;
    Vector2f left_top = position - size * 0.5f;
    Vector2f right_bottom = position + size * 0.5f;

    Mat4 proj = Mat4::ortho(left_top.x, right_bottom.x, right_bottom.y, left_top.y, -1.0f, 1.0f);

    m_shader->bind();
    m_shader->set_mat4("u_Projection", proj);
    // Removed unbind so that it stays bound during drawing.
}

void Renderer2D::begin() {
    m_stats = m_stats_current;
    m_stats_current = {0, 0, 0};

    m_vertex_count = 0;
    m_index_count = 0;
    m_texture_count = 1;
    m_textures[0] = m_white_texture.get();

    m_shader->bind();
}

void Renderer2D::end() {
    flush();
    m_shader->unbind();
}

void Renderer2D::flush() {
    if (m_index_count == 0) return;

    for (u32 i = 0; i < m_texture_count; i++) {
        m_textures[i]->bind(i);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertex_count * sizeof(Vertex2D), m_vertices.data());

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    m_stats_current.draw_calls++;
    m_stats_current.vertex_count += m_vertex_count;
    m_stats_current.quad_count += m_index_count / 6;

    m_vertex_count = 0;
    m_index_count = 0;
    m_texture_count = 1;
}

f32 Renderer2D::get_texture_index(const Texture2D* tex) {
    if (!tex) return 0.0f;
    for (u32 i = 0; i < m_texture_count; i++) {
        if (m_textures[i] == tex) return static_cast<f32>(i);
    }
    if (m_texture_count >= MAX_TEXTURE_SLOTS) {
        flush();
    }
    m_textures[m_texture_count] = tex;
    return static_cast<f32>(m_texture_count++);
}

void Renderer2D::push_quad(const Vector2f* positions, const Vector2f* uvs, Color color, f32 tex_index) {
    if (m_vertex_count + 4 > MAX_VERTICES || m_index_count + 6 > MAX_INDICES) {
        flush();
    }

    for (int i = 0; i < 4; i++) {
        m_vertices[m_vertex_count + i] = {positions[i], uvs[i], color, tex_index};
    }

    m_vertex_count += 4;
    m_index_count += 6;
}

void Renderer2D::push_triangle(const Vector2f* positions, const Vector2f* uvs, Color color, f32 tex_index) {
    if (m_vertex_count + 4 > MAX_VERTICES || m_index_count + 6 > MAX_INDICES) {
        flush();
    }

    for (int i = 0; i < 3; i++) {
        m_vertices[m_vertex_count + i] = {positions[i], uvs[i], color, tex_index};
    }
    m_vertices[m_vertex_count + 3] = m_vertices[m_vertex_count + 2]; // Degenerate quad

    m_vertex_count += 4;
    m_index_count += 6;
}

void Renderer2D::draw_rect(Vector2f position, Vector2f size, Color color, f32 rotation, Vector2f origin) {
    Vector2f uvs[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    Vector2f positions[4] = {
        {0.0f, 0.0f},
        {size.x, 0.0f},
        {size.x, size.y},
        {0.0f, size.y}
    };

    Vector2f center = {size.x * origin.x, size.y * origin.y};

    f32 s = std::sin(rotation);
    f32 c = std::cos(rotation);

    for (int i = 0; i < 4; i++) {
        Vector2f p = positions[i] - center;
        positions[i] = {
            p.x * c - p.y * s + position.x,
            p.x * s + p.y * c + position.y
        };
    }

    push_quad(positions, uvs, color, 0.0f);
}

void Renderer2D::draw_rect_outline(Vector2f position, Vector2f size, Color color, f32 thickness, f32 rotation) {
    Vector2f p1 = position;
    Vector2f p2 = position + Vector2f(size.x, 0);
    Vector2f p3 = position + size;
    Vector2f p4 = position + Vector2f(0, size.y);

    if (rotation != 0.0f) {
        Vector2f center = size * 0.5f;
        auto rot = [rotation](Vector2f p) { return p.rotated(rotation); };
        p1 = rot(p1 - position - center) + position + center;
        p2 = rot(p2 - position - center) + position + center;
        p3 = rot(p3 - position - center) + position + center;
        p4 = rot(p4 - position - center) + position + center;
    }

    draw_line(p1, p2, color, thickness);
    draw_line(p2, p3, color, thickness);
    draw_line(p3, p4, color, thickness);
    draw_line(p4, p1, color, thickness);
}

void Renderer2D::draw_sprite(const Texture2D& tex, Vector2f position, Vector2f size, Color tint, f32 rotation, Vector2f origin) {
    Vector2f uvs[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    Vector2f positions[4] = {
        {0.0f, 0.0f},
        {size.x, 0.0f},
        {size.x, size.y},
        {0.0f, size.y}
    };

    Vector2f center = {size.x * origin.x, size.y * origin.y};
    f32 s = std::sin(rotation);
    f32 c = std::cos(rotation);

    for (int i = 0; i < 4; i++) {
        Vector2f p = positions[i] - center;
        positions[i] = {
            p.x * c - p.y * s + position.x,
            p.x * s + p.y * c + position.y
        };
    }

    push_quad(positions, uvs, tint, get_texture_index(&tex));
}

void Renderer2D::draw_sprite_region(const Texture2D& tex, Rect2f src_rect, Vector2f position, Vector2f size, Color tint, f32 rotation) {
    f32 tex_w = static_cast<f32>(tex.get_width());
    f32 tex_h = static_cast<f32>(tex.get_height());

    Vector2f u_min = src_rect.position / Vector2f(tex_w, tex_h);
    Vector2f u_max = (src_rect.position + src_rect.size) / Vector2f(tex_w, tex_h);

    Vector2f uvs[4] = {
        {u_min.x, u_min.y},
        {u_max.x, u_min.y},
        {u_max.x, u_max.y},
        {u_min.x, u_max.y}
    };

    Vector2f positions[4] = {
        {0.0f, 0.0f},
        {size.x, 0.0f},
        {size.x, size.y},
        {0.0f, size.y}
    };

    Vector2f center = size * 0.5f;
    f32 s = std::sin(rotation);
    f32 c = std::cos(rotation);

    for (int i = 0; i < 4; i++) {
        Vector2f p = positions[i] - center;
        positions[i] = {
            p.x * c - p.y * s + position.x,
            p.x * s + p.y * c + position.y
        };
    }

    push_quad(positions, uvs, tint, get_texture_index(&tex));
}

void Renderer2D::draw_circle(Vector2f center, f32 radius, Color color, i32 segments) {
    if (segments < 3) return;
    
    Vector2f p0 = center;
    Vector2f uv0 = {0.5f, 0.5f};

    f32 angle_step = 6.2831853f / static_cast<f32>(segments);
    Vector2f p1 = center + Vector2f(std::cos(0.0f), std::sin(0.0f)) * radius;

    for (int i = 0; i < segments; i++) {
        f32 a2 = (i + 1) * angle_step;
        Vector2f p2 = center + Vector2f(std::cos(a2), std::sin(a2)) * radius;

        Vector2f positions[3] = {p0, p1, p2};
        Vector2f uvs[3] = {uv0, {0.5f, 0.5f}, {0.5f, 0.5f}};

        push_triangle(positions, uvs, color, 0.0f);
        p1 = p2;
    }
}

void Renderer2D::draw_circle_outline(Vector2f center, f32 radius, Color color, f32 thickness, i32 segments) {
    if (segments < 3) return;
    f32 angle_step = 6.2831853f / static_cast<f32>(segments);
    Vector2f prev = center + Vector2f(std::cos(0.0f), std::sin(0.0f)) * radius;

    for (int i = 1; i <= segments; i++) {
        f32 a = i * angle_step;
        Vector2f curr = center + Vector2f(std::cos(a), std::sin(a)) * radius;
        draw_line(prev, curr, color, thickness);
        prev = curr;
    }
}

void Renderer2D::draw_line(Vector2f a, Vector2f b, Color color, f32 thickness) {
    Vector2f dir = (b - a);
    f32 len = dir.length();
    if (len == 0.0f) return;
    dir = dir / len;
    
    Vector2f normal = {-dir.y, dir.x};
    Vector2f offset = normal * (thickness * 0.5f);

    Vector2f positions[4] = {
        a + offset,
        b + offset,
        b - offset,
        a - offset
    };
    Vector2f uvs[4] = {{0,0}, {1,0}, {1,1}, {0,1}};

    push_quad(positions, uvs, color, 0.0f);
}

void Renderer2D::draw_polygon(std::span<const Vector2f> points, Color color) {
    if (points.size() < 3) return;

    Vector2f p0 = points[0];
    Vector2f uvs[3] = {{0.5f, 0.5f}, {0.5f, 0.5f}, {0.5f, 0.5f}};

    for (usize i = 1; i < points.size() - 1; i++) {
        Vector2f positions[3] = {p0, points[i], points[i + 1]};
        push_triangle(positions, uvs, color, 0.0f);
    }
}

void Renderer2D::draw_polygon_outline(std::span<const Vector2f> points, Color color, f32 thickness) {
    if (points.size() < 2) return;
    for (usize i = 0; i < points.size() - 1; i++) {
        draw_line(points[i], points[i + 1], color, thickness);
    }
    draw_line(points.back(), points.front(), color, thickness);
}

void Renderer2D::draw_text(std::string_view text, Vector2f position, Color color, f32 size) {
    (void)text; (void)position; (void)color; (void)size;
    // Stub
}

} // namespace nova

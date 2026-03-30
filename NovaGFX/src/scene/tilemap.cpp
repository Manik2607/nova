#include "nova/scene/tilemap.hpp"
#include <algorithm>
#include <cmath>

namespace nova {

const Tile Tilemap::EMPTY_TILE = {};

void Tilemap::create(i32 width, i32 height, f32 tile_size, i32 num_layers) {
    m_width = width;
    m_height = height;
    m_tile_size = tile_size;
    m_num_layers = num_layers;

    m_layers.resize(static_cast<usize>(num_layers));
    for (auto& layer : m_layers) {
        layer.resize(static_cast<usize>(width * height));
    }

    // Default color palette
    m_tile_colors.resize(256, Color::MAGENTA()); // bright magenta = unmapped
    m_tile_colors[0] = Color::TRANSPARENT();     // 0 = empty/transparent
}

void Tilemap::set_tile(i32 layer, i32 x, i32 y, u16 tile_id, SurfaceType surface) {
    if (layer < 0 || layer >= m_num_layers) return;
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;

    auto& tile = m_layers[static_cast<usize>(layer)][static_cast<usize>(y * m_width + x)];
    tile.tile_id = tile_id;
    tile.surface = surface;
}

const Tile& Tilemap::get_tile(i32 layer, i32 x, i32 y) const {
    if (layer < 0 || layer >= m_num_layers) return EMPTY_TILE;
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return EMPTY_TILE;
    return m_layers[static_cast<usize>(layer)][static_cast<usize>(y * m_width + x)];
}

SurfaceType Tilemap::get_surface_at(Vector2f world_pos) const {
    Vector2i grid = world_to_grid(world_pos);
    // Check top layer first, fall through to base layer
    for (i32 l = m_num_layers - 1; l >= 0; l--) {
        const Tile& t = get_tile(l, grid.x, grid.y);
        if (t.surface != SurfaceType::NONE) return t.surface;
    }
    return SurfaceType::GRASS; // default off-map
}

void Tilemap::draw(Renderer2D& renderer, Rect2f camera_bounds) {
    for (i32 l = 0; l < m_num_layers; l++) {
        draw_layer(renderer, camera_bounds, l);
    }
}

void Tilemap::draw_layer(Renderer2D& renderer, Rect2f camera_bounds, i32 layer) {
    if (layer < 0 || layer >= m_num_layers) return;

    // Compute visible tile range (frustum culling)
    i32 x_min = static_cast<i32>(std::floor(camera_bounds.position.x / m_tile_size));
    i32 y_min = static_cast<i32>(std::floor(camera_bounds.position.y / m_tile_size));
    i32 x_max = static_cast<i32>(std::ceil((camera_bounds.position.x + camera_bounds.size.x) / m_tile_size));
    i32 y_max = static_cast<i32>(std::ceil((camera_bounds.position.y + camera_bounds.size.y) / m_tile_size));

    x_min = std::max(0, x_min);
    y_min = std::max(0, y_min);
    x_max = std::min(m_width, x_max);
    y_max = std::min(m_height, y_max);

    for (i32 y = y_min; y < y_max; y++) {
        for (i32 x = x_min; x < x_max; x++) {
            const Tile& tile = m_layers[static_cast<usize>(layer)][static_cast<usize>(y * m_width + x)];
            if (tile.tile_id == 0) continue;

            u16 cid = tile.tile_id;
            Color color = (cid < m_tile_colors.size()) ? m_tile_colors[cid] : Color::MAGENTA();
            if (color.a <= 0.0f) continue;

            Vector2f pos = {x * m_tile_size, y * m_tile_size};
            renderer.draw_rect(pos, {m_tile_size, m_tile_size}, color);
        }
    }
}

void Tilemap::set_tile_color(u16 tile_id, Color color) {
    if (static_cast<usize>(tile_id) >= m_tile_colors.size()) {
        m_tile_colors.resize(static_cast<usize>(tile_id) + 1, Color::MAGENTA());
    }
    m_tile_colors[static_cast<usize>(tile_id)] = color;
}

Vector2i Tilemap::world_to_grid(Vector2f world_pos) const {
    return {
        static_cast<i32>(std::floor(world_pos.x / m_tile_size)),
        static_cast<i32>(std::floor(world_pos.y / m_tile_size))
    };
}

Vector2f Tilemap::grid_to_world(i32 x, i32 y) const {
    return {x * m_tile_size, y * m_tile_size};
}

} // namespace nova

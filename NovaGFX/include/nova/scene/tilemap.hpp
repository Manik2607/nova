/**
 * Tilemap — Engine-level tilemap renderer with layers and surface metadata.
 *
 * API Surface:
 *   Tilemap map;
 *   map.create(width, height, tile_size);
 *   map.set_tile(layer, x, y, tile_id, surface_type);
 *   map.draw(renderer, camera_bounds);   // frustum-culled
 *   SurfaceType s = map.get_surface_at(world_pos);
 *
 * Surface types affect gameplay (grip coefficients).
 */
#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/rect2.hpp"
#include "nova/math/color.hpp"
#include "nova/renderer/renderer2d.hpp"
#include <vector>
#include <string>

namespace nova {

enum class SurfaceType : u8 {
    TARMAC = 0,
    DIRT,
    GRASS,
    GRAVEL,
    WALL,
    NONE
};

struct Tile {
    u16 tile_id = 0;          // 0 = empty
    SurfaceType surface = SurfaceType::NONE;
};

class Tilemap {
public:
    Tilemap() = default;

    /// Create an empty tilemap.
    void create(i32 width, i32 height, f32 tile_size, i32 num_layers = 1);

    /// Set a tile at grid coordinates.
    void set_tile(i32 layer, i32 x, i32 y, u16 tile_id, SurfaceType surface = SurfaceType::NONE);

    /// Get the tile at grid coordinates.
    const Tile& get_tile(i32 layer, i32 x, i32 y) const;

    /// Get surface type at world position.
    SurfaceType get_surface_at(Vector2f world_pos) const;

    /// Draw all layers, frustum-culled to camera bounds.
    void draw(Renderer2D& renderer, Rect2f camera_bounds);

    /// Draw a specific layer.
    void draw_layer(Renderer2D& renderer, Rect2f camera_bounds, i32 layer);

    /// Map a tile_id to a color (for procedural rendering without a tileset texture).
    void set_tile_color(u16 tile_id, Color color);

    i32 get_width() const { return m_width; }
    i32 get_height() const { return m_height; }
    f32 get_tile_size() const { return m_tile_size; }
    i32 get_num_layers() const { return m_num_layers; }

    /// Convert world position to grid coordinates.
    Vector2i world_to_grid(Vector2f world_pos) const;
    /// Convert grid coordinates to world position (top-left corner of tile).
    Vector2f grid_to_world(i32 x, i32 y) const;

private:
    i32 m_width = 0;
    i32 m_height = 0;
    f32 m_tile_size = 64.0f;
    i32 m_num_layers = 1;

    // layers[layer][y * width + x]
    std::vector<std::vector<Tile>> m_layers;

    // tile_id → color mapping for procedural rendering
    std::vector<Color> m_tile_colors;

    static const Tile EMPTY_TILE;
};

} // namespace nova

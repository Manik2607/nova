#pragma once
#include <nova/nova.hpp>
#include <box2d/box2d.h>
#include "nova/scene/tilemap.hpp"
#include <vector>

using namespace nova;

/// Procedurally generates a closed-loop race track using the tilemap system.
class Track {
public:
    Track() = default;

    /// Generate the track and populate the tilemap + Box2D walls.
    void generate(b2World* world, i32 seed = 42);

    /// Draw the track.
    void draw(Renderer2D& renderer, Rect2f camera_bounds);

    /// Get the tilemap (for surface lookups).
    Tilemap& get_tilemap() { return m_tilemap; }

    /// Get the starting position and angle.
    Vector2f get_start_position() const { return m_start_pos; }
    f32 get_start_angle() const { return m_start_angle; }

    /// Get track centerline points (for reference/minimap).
    const std::vector<Vector2f>& get_centerline() const { return m_centerline; }

private:
    void build_tilemap();
    void build_walls(b2World* world);

    Tilemap m_tilemap;

    // Track layout
    std::vector<Vector2f> m_centerline;    // centerline points
    f32 m_track_width = 180.0f;            // half-width from center to edge
    Vector2f m_start_pos;
    f32 m_start_angle = 0.0f;

    // Tilemap constants
    static constexpr f32 TILE_SIZE = 32.0f;
    static constexpr i32 MAP_WIDTH = 200;
    static constexpr i32 MAP_HEIGHT = 200;

    // Tile IDs
    static constexpr u16 TILE_GRASS   = 1;
    static constexpr u16 TILE_TARMAC  = 2;
    static constexpr u16 TILE_CURB    = 3;
    static constexpr u16 TILE_DIRT    = 4;
    static constexpr u16 TILE_START   = 5;
};

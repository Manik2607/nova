#include "track.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Track::generate(b2World* world, i32 seed) {
    (void)seed;
    // Generate a closed-loop track as a set of centerline points.
    // We create an oval-ish track with interesting curves.
    m_centerline.clear();

    // Track center is at the middle of the tilemap
    Vector2f center = {MAP_WIDTH * TILE_SIZE * 0.5f, MAP_HEIGHT * TILE_SIZE * 0.5f};
    f32 base_radius_x = 2200.0f;
    f32 base_radius_y = 1400.0f;

    i32 num_points = 120;
    for (i32 i = 0; i < num_points; i++) {
        f32 t = static_cast<f32>(i) / static_cast<f32>(num_points) * 2.0f * static_cast<f32>(M_PI);

        // Add some wobble for interesting track shape
        f32 wobble = 1.0f + 0.15f * std::sin(t * 3.0f) + 0.1f * std::cos(t * 5.0f + 1.0f)
                    + 0.08f * std::sin(t * 7.0f + 2.5f);

        f32 rx = base_radius_x * wobble;
        f32 ry = base_radius_y * wobble;

        Vector2f point = center + Vector2f(std::cos(t) * rx, std::sin(t) * ry);
        m_centerline.push_back(point);
    }

    // Start position: first centerline point, facing toward second
    m_start_pos = m_centerline[0];
    Vector2f to_next = (m_centerline[1] - m_centerline[0]).normalized();
    m_start_angle = std::atan2(to_next.x, -to_next.y); // convert to our angle convention

    build_tilemap();
    build_walls(world);
}

void Track::build_tilemap() {
    // Create the tilemap with 2 layers: surface + decorations
    m_tilemap.create(MAP_WIDTH, MAP_HEIGHT, TILE_SIZE, 2);

    // Set tile colors (neon-synthwave palette)
    m_tilemap.set_tile_color(TILE_GRASS,  Color(0.02f, 0.08f, 0.05f, 1.0f)); // dark green-black
    m_tilemap.set_tile_color(TILE_TARMAC, Color(0.08f, 0.06f, 0.12f, 1.0f)); // dark purple-gray
    m_tilemap.set_tile_color(TILE_CURB,   Color(0.9f, 0.1f, 0.4f, 1.0f));    // neon pink curb
    m_tilemap.set_tile_color(TILE_DIRT,   Color(0.15f, 0.1f, 0.05f, 1.0f));   // dark brown
    m_tilemap.set_tile_color(TILE_START,  Color(0.15f, 0.08f, 0.15f, 1.0f));  // slightly lighter tarmac

    // Fill everything with grass first
    for (i32 y = 0; y < MAP_HEIGHT; y++) {
        for (i32 x = 0; x < MAP_WIDTH; x++) {
            m_tilemap.set_tile(0, x, y, TILE_GRASS, SurfaceType::GRASS);
        }
    }

    // Paint the track: for each tile, check distance to closest centerline segment
    for (i32 y = 0; y < MAP_HEIGHT; y++) {
        for (i32 x = 0; x < MAP_WIDTH; x++) {
            Vector2f tile_center = {x * TILE_SIZE + TILE_SIZE * 0.5f, y * TILE_SIZE + TILE_SIZE * 0.5f};

            // Find minimum distance to centerline
            f32 min_dist = 1e9f;
            for (usize i = 0; i < m_centerline.size(); i++) {
                usize next = (i + 1) % m_centerline.size();
                Vector2f a = m_centerline[i];
                Vector2f b = m_centerline[next];

                // Point-to-segment distance
                Vector2f ab = b - a;
                f32 ab_len2 = ab.length_squared();
                if (ab_len2 < 0.001f) continue;
                f32 t = std::clamp((tile_center - a).dot(ab) / ab_len2, 0.0f, 1.0f);
                Vector2f closest = a + ab * t;
                f32 dist = (tile_center - closest).length();
                min_dist = std::min(min_dist, dist);
            }

            if (min_dist < m_track_width - 20.0f) {
                // Road surface
                m_tilemap.set_tile(0, x, y, TILE_TARMAC, SurfaceType::TARMAC);
            } else if (min_dist < m_track_width) {
                // Curb zone
                // Alternate curb pattern
                if ((x + y) % 2 == 0) {
                    m_tilemap.set_tile(0, x, y, TILE_CURB, SurfaceType::TARMAC);
                } else {
                    m_tilemap.set_tile(0, x, y, TILE_TARMAC, SurfaceType::TARMAC);
                }
            } else if (min_dist < m_track_width + 40.0f) {
                // Dirt shoulder
                m_tilemap.set_tile(0, x, y, TILE_DIRT, SurfaceType::DIRT);
            }
            // else remains grass
        }
    }

    // Mark the start/finish line area
    Vector2f start_grid = {
        m_start_pos.x / TILE_SIZE,
        m_start_pos.y / TILE_SIZE
    };
    for (i32 dy = -2; dy <= 2; dy++) {
        for (i32 dx = -3; dx <= 3; dx++) {
            i32 tx = static_cast<i32>(start_grid.x) + dx;
            i32 ty = static_cast<i32>(start_grid.y) + dy;
            if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) {
                // Checkerboard start line on decoration layer
                if ((tx + ty) % 2 == 0) {
                    m_tilemap.set_tile(1, tx, ty, TILE_START, SurfaceType::NONE);
                }
            }
        }
    }
}

void Track::build_walls(b2World* world) {
    // Create wall bodies along the outer and inner edges of the track.
    // We sample the centerline and place edge chain segments.

    // We'll create edge bodies for the inner and outer boundaries.
    // For simplicity, use edge shapes between consecutive boundary points.
    f32 wall_offset = m_track_width + 5.0f;

    auto make_wall_chain = [&](f32 offset) {
        usize n = m_centerline.size();
        for (usize i = 0; i < n; i++) {
            usize next = (i + 1) % n;
            Vector2f a = m_centerline[i];
            Vector2f b = m_centerline[next];

            // Normal to segment (pointing outward for positive offset)
            Vector2f dir = (b - a).normalized();
            Vector2f normal = {-dir.y, dir.x};

            Vector2f wa = a + normal * offset;
            Vector2f wb = b + normal * offset;

            b2BodyDef bd;
            bd.type = b2_staticBody;
            b2Body* body = world->CreateBody(&bd);

            b2EdgeShape edge;
            edge.SetTwoSided(b2Vec2(wa.x, wa.y), b2Vec2(wb.x, wb.y));

            b2FixtureDef fd;
            fd.shape = &edge;
            fd.friction = 0.5f;
            fd.restitution = 0.3f;
            body->CreateFixture(&fd);
        }
    };

    // Outer wall
    make_wall_chain(wall_offset);
    // Inner wall (negative offset)
    make_wall_chain(-wall_offset);
}

void Track::draw(Renderer2D& renderer, Rect2f camera_bounds) {
    m_tilemap.draw(renderer, camera_bounds);

    usize n = m_centerline.size();

    // Draw neon edge glow lines along track boundaries
    for (usize i = 0; i < n; i++) {
        usize next = (i + 1) % n;
        Vector2f a = m_centerline[i];
        Vector2f b = m_centerline[next];

        // Check if roughly in camera view (rough frustum cull)
        f32 bx = camera_bounds.position.x;
        f32 by = camera_bounds.position.y;
        f32 bx2 = bx + camera_bounds.size.x;
        f32 by2 = by + camera_bounds.size.y;
        if (a.x < bx - 400 && b.x < bx - 400) continue;
        if (a.x > bx2 + 400 && b.x > bx2 + 400) continue;
        if (a.y < by - 400 && b.y < by - 400) continue;
        if (a.y > by2 + 400 && b.y > by2 + 400) continue;

        Vector2f dir = (b - a).normalized();
        Vector2f normal = {-dir.y, dir.x};

        // Outer edge glow (neon pink)
        f32 edge_offset = m_track_width - 15.0f;
        Vector2f oa = a + normal * edge_offset;
        Vector2f ob = b + normal * edge_offset;
        renderer.draw_line(oa, ob, Color(1.0f, 0.1f, 0.5f, 0.4f), 2.5f);

        // Inner edge glow (neon cyan)
        Vector2f ia = a - normal * edge_offset;
        Vector2f ib = b - normal * edge_offset;
        renderer.draw_line(ia, ib, Color(0.0f, 0.8f, 1.0f, 0.35f), 2.5f);

        // Dashed center line
        if (i % 6 < 3) {
            Color center_color(1.0f, 1.0f, 1.0f, 0.08f);
            renderer.draw_line(a, b, center_color, 1.5f);
        }
    }
}

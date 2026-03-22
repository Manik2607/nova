#include "terrain.hpp"
#include <cmath>

using namespace nova;

extern const float PIXELS_PER_METER;

TerrainGenerator::TerrainGenerator(b2World* world) : m_world(world) {
    // Generate initial chunks
    while (m_last_generated_x < MAX_GENERATE_AHEAD) {
        generate_chunk();
    }
}

float TerrainGenerator::get_height(float x) {
    // Mix of sine waves for rolling hills
    float y = 0.0f;
    y += std::sin(x * 0.1f) * 2.5f;
    y += std::sin(x * 0.3f) * 0.8f;
    y += std::sin(x * 0.05f) * 4.0f;

    // Add procedural ramps
    float ramp_mod = std::fmod(x, 150.0f);
    if (ramp_mod < 0) ramp_mod += 150.0f; // Handle negative x

    if (ramp_mod > 100.0f && ramp_mod <= 120.0f) {
        y -= (ramp_mod - 100.0f) * 0.8f; // Ramp up gradually (negative Y is up)
    } else if (ramp_mod > 120.0f && ramp_mod <= 122.0f) {
        y -= 16.0f; // Flat top lip
    }
    
    return y + 8.0f; // Base offset
}

void TerrainGenerator::generate_chunk() {
    TerrainChunk chunk;
    chunk.start_x = m_last_generated_x;
    chunk.end_x = chunk.start_x + CHUNK_WIDTH;
    
    b2BodyDef bodyDef;
    chunk.body = m_world->CreateBody(&bodyDef);
    
    std::vector<b2Vec2> physics_vertices;
    
    for (float x = chunk.start_x; x <= chunk.end_x + 0.01f; x += VERTEX_STEP) {
        float y = get_height(x);
        physics_vertices.push_back(b2Vec2(x, y));
        chunk.visual_points.push_back({x * PIXELS_PER_METER, y * PIXELS_PER_METER});
    }
    
    b2ChainShape chain;
    chain.CreateChain(physics_vertices.data(), physics_vertices.size(), 
                      b2Vec2(physics_vertices.front().x - VERTEX_STEP, get_height(physics_vertices.front().x - VERTEX_STEP)), 
                      b2Vec2(physics_vertices.back().x + VERTEX_STEP, get_height(physics_vertices.back().x + VERTEX_STEP)));
    
    b2FixtureDef fd;
    fd.shape = &chain;
    fd.friction = 0.8f;   // Sticky enough for tires
    fd.restitution = 0.1f;
    chunk.body->CreateFixture(&fd);
    
    // Occasionally spawn an obstacle crate
    if ((int)chunk.start_x % 30 == 0 && chunk.start_x > 10.0f) {
        b2BodyDef crateDef;
        crateDef.type = b2_dynamicBody;
        crateDef.position.Set(chunk.start_x + CHUNK_WIDTH / 2.0f, get_height(chunk.start_x + CHUNK_WIDTH / 2.0f) - 2.0f);
        b2Body* crate = m_world->CreateBody(&crateDef);
        
        b2PolygonShape crateShape;
        crateShape.SetAsBox(0.4f, 0.4f);
        
        b2FixtureDef cfd;
        cfd.shape = &crateShape;
        cfd.density = 0.5f;
        cfd.friction = 0.6f;
        crate->CreateFixture(&cfd);
    }
    
    m_chunks.push_back(chunk);
    m_last_generated_x = chunk.end_x;
}

void TerrainGenerator::update(float camera_x) {
    float cam_m = camera_x / PIXELS_PER_METER;
    
    // Delete old chunks
    while (!m_chunks.empty() && m_chunks.front().end_x < cam_m - MAX_KEEP_BEHIND) {
        m_world->DestroyBody(m_chunks.front().body);
        m_chunks.pop_front();
    }
    
    // Generate new chunks ahead
    while (m_last_generated_x < cam_m + MAX_GENERATE_AHEAD) {
        generate_chunk();
    }
}

void TerrainGenerator::draw(Renderer2D& renderer) {
    // We will draw the terrain as filled polygons dropping down to the bottom of the screen.
    const float BOTTOM_Y = 3000.0f; // Some deep Y coordinate in pixels
    
    Color ground_top(0.25f, 0.5f, 0.25f, 1.0f);   // Grass
    Color ground_bottom(0.15f, 0.1f, 0.05f, 1.0f); // Dirt
    Color line_color(0.1f, 0.4f, 0.1f, 1.0f);
    
    for (const auto& chunk : m_chunks) {
        if (chunk.visual_points.size() < 2) continue;
        
        // Draw lines along the edge
        for (usize i = 0; i < chunk.visual_points.size() - 1; ++i) {
            renderer.draw_line(chunk.visual_points[i], chunk.visual_points[i+1], line_color, 4.0f);
        }
        
        // Fill underneath the hills by faking thick lines vertically
        for (usize i = 0; i < chunk.visual_points.size() - 1; ++i) {
            std::vector<Vector2f> quad = {
                chunk.visual_points[i],
                chunk.visual_points[i+1],
                {chunk.visual_points[i+1].x, BOTTOM_Y},
                {chunk.visual_points[i].x, BOTTOM_Y}
            };
            renderer.draw_polygon(quad, ground_bottom);
        }
    }
}

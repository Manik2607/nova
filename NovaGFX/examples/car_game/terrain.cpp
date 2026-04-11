#include "terrain.hpp"
#include <cmath>
#include <algorithm>

using namespace nova;

extern const float PIXELS_PER_METER;

namespace {
    float hash(float n) {
        float f = std::sin(n) * 43758.5453123f;
        return f - std::floor(f);
    }
    
    float noise(float x) {
        float i = std::floor(x);
        float f = x - i;
        float u = f * f * (3.0f - 2.0f * f); 
        return hash(i) * (1.0f - u) + hash(i + 1.0f) * u;
    }

    float fbm(float x, int octaves) {
        float value = 0.0f;
        float amplitude = 0.5f;
        for (int i = 0; i < octaves; ++i) {
            value += amplitude * noise(x);
            x = x * 2.0f + 10.0f;
            amplitude *= 0.5f;
        }
        return value;
    }
}

TerrainGenerator::TerrainGenerator(b2World* world) : m_world(world) {
    m_dirt_tex = std::make_shared<Texture2D>("examples/car_game/Images/dirt.png");
    m_grass_tex = std::make_shared<Texture2D>("examples/car_game/Images/grass.png");
    // Generate initial chunks
    while (m_last_generated_x < MAX_GENERATE_AHEAD) {
        generate_chunk();
    }
}

float TerrainGenerator::get_height(float x) {
    float y = 0.0f;
    
    // Very low frequency, large rolling curvy hills (SOFTER)
    y += std::sin(x * 0.015f) * 10.0f;
    y += std::sin(x * 0.03f) * 5.0f;
    
    // Add noise-based terrain macro variations
    y += (fbm(x * 0.03f * roughness, 3) - 0.5f) * 10.0f * bumpiness;

    // Mask for micro bumps (only appear in some places)
    float bump_density = noise(x * 0.05f * roughness + 123.4f);
    float bump_mask = std::max(0.0f, bump_density - 0.6f) * 2.5f;

    // Add micro bumps, modulated by the mask
    float bumps = fbm(x * 1.0f * roughness, 2);
    y += (bumps - 0.5f) * 2.0f * bump_mask * bumpiness;

    // Extra sharp micro details sparsely scattered
    float sharp_bump_mask = std::max(0.0f, noise(x * 0.15f * roughness + 456.7f) - 0.8f) * 5.0f;
    y += (noise(x * 3.0f * roughness) - 0.5f) * 1.0f * sharp_bump_mask * bumpiness;
    
    return y + 10.0f; // Base offset
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

void TerrainGenerator::reset(float start_x) {
    // Destroy all existing chunk bodies
    for (auto& chunk : m_chunks) {
        m_world->DestroyBody(chunk.body);
    }
    m_chunks.clear();

    // Restart generation from a little behind the car
    float start_m = start_x / PIXELS_PER_METER - CHUNK_WIDTH;
    m_last_generated_x = start_m;

    // Regenerate chunks ahead
    while (m_last_generated_x < start_m + MAX_GENERATE_AHEAD + CHUNK_WIDTH) {
        generate_chunk();
    }
}

void TerrainGenerator::draw(Renderer2D& renderer) {
    const float BOTTOM_Y = 3000.0f; // Deep Y coordinate in pixels
    const float TOP_LAYER_THICKNESS = 40.0f; // Pixels
    
    Color ground_top(0.3f, 0.7f, 0.2f, 1.0f);   // Bright Green grass
    Color ground_bottom(0.25f, 0.15f, 0.05f, 1.0f); // Mud
    
    for (const auto& chunk : m_chunks) {
        if (chunk.visual_points.size() < 2) continue;
        
        for (usize i = 0; i < chunk.visual_points.size() - 1; ++i) {
            // Draw Grass Top Layer with tiling vector texture
            Vector2f grass_quad[4] = {
                chunk.visual_points[i],
                chunk.visual_points[i+1],
                {chunk.visual_points[i+1].x, chunk.visual_points[i+1].y + TOP_LAYER_THICKNESS},
                {chunk.visual_points[i].x, chunk.visual_points[i].y + TOP_LAYER_THICKNESS}
            };

            const float grass_tex_size = 128.0f; // Scale factor for grass tiling
            Vector2f grass_uv[4] = {
                { grass_quad[0].x / grass_tex_size, 0.0f },
                { grass_quad[1].x / grass_tex_size, 0.0f },
                { grass_quad[2].x / grass_tex_size, 1.0f },
                { grass_quad[3].x / grass_tex_size, 1.0f }
            };

            if (m_grass_tex) {
                renderer.draw_textured_quad(*m_grass_tex, grass_quad, grass_uv);
            } else {
                renderer.draw_polygon(std::vector<Vector2f>(grass_quad, grass_quad + 4), ground_top);
            }

            // Draw Mud Bottom Layer with tiling dirt texture
            Vector2f pos_quad[4] = {
                {chunk.visual_points[i].x, chunk.visual_points[i].y + TOP_LAYER_THICKNESS},
                {chunk.visual_points[i+1].x, chunk.visual_points[i+1].y + TOP_LAYER_THICKNESS},
                {chunk.visual_points[i+1].x, BOTTOM_Y},
                {chunk.visual_points[i].x, BOTTOM_Y}
            };

            // Calculate world-space UVs for tiling
            const float tex_size = 512.0f; // Scale factor for tiling
            Vector2f uv_quad[4] = {
                { pos_quad[0].x / tex_size, pos_quad[0].y / tex_size },
                { pos_quad[1].x / tex_size, pos_quad[1].y / tex_size },
                { pos_quad[2].x / tex_size, pos_quad[2].y / tex_size },
                { pos_quad[3].x / tex_size, pos_quad[3].y / tex_size }
            };

            if (m_dirt_tex) {
                renderer.draw_textured_quad(*m_dirt_tex, pos_quad, uv_quad);
            } else {
                renderer.draw_polygon(std::vector<Vector2f>(pos_quad, pos_quad + 4), ground_bottom);
            }
        }
    }
}

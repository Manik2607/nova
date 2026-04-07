#pragma once
#include <nova/nova.hpp>
#include <box2d/box2d.h>
#include <vector>
#include <deque>

struct TerrainChunk {
    b2Body* body;
    float start_x;
    float end_x;
    std::vector<nova::Vector2f> visual_points;
};

class TerrainGenerator {
public:
    TerrainGenerator(b2World* world);
    
    // Generates chunks ahead of the given x position, and destroys chunks that are too far behind.
    void update(float camera_x);
    void draw(nova::Renderer2D& renderer);
    void reset(float start_x);

    float bumpiness = 1.0f;
    float roughness = 1.5f;

private:
    b2World* m_world;
    std::deque<TerrainChunk> m_chunks;
    
    float m_last_generated_x = -10.0f;
    const float CHUNK_WIDTH = 20.0f; // meters
    const float VERTEX_STEP = 0.5f;  // meters between vertices
    const float MAX_KEEP_BEHIND = 30000.0f; // meters to keep behind camera
    const float MAX_GENERATE_AHEAD = 40.0f; // meters to generate ahead
    
    float get_height(float x);
    void generate_chunk();
};

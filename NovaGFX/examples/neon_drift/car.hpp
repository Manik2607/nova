#pragma once
#include <nova/nova.hpp>
#include <box2d/box2d.h>
#include "nova/scene/tilemap.hpp"

using namespace nova;

/// Surface grip coefficients mapped from SurfaceType.
inline f32 get_grip_for_surface(SurfaceType s) {
    switch (s) {
        case SurfaceType::TARMAC:  return 1.0f;
        case SurfaceType::DIRT:    return 0.6f;
        case SurfaceType::GRASS:   return 0.4f;
        case SurfaceType::GRAVEL:  return 0.5f;
        default: return 0.3f;
    }
}

/// Top-down drift car using Box2D with zero gravity.
class Car {
public:
    Car(b2World* world, Vector2f start_pos, f32 start_angle = 0.0f);

    void update(f32 dt, f32 surface_grip = 1.0f);
    void draw(Renderer2D& renderer);

    Vector2f get_position() const;
    Vector2f get_velocity() const;
    f32 get_angle() const;
    f32 get_speed() const;
    f32 get_slip_angle() const;
    bool is_drifting() const;

    /// Get forward direction (unit vector).
    Vector2f get_forward() const;
    /// Get right direction (unit vector).
    Vector2f get_right() const;

    /// Get position of rear wheels in world space (for tire marks).
    void get_rear_wheel_positions(Vector2f& left, Vector2f& right) const;

    b2Body* get_body() const { return m_body; }

private:
    void apply_drive_force(f32 dt);
    void apply_steering(f32 dt);
    void apply_friction(f32 dt, f32 surface_grip);

    b2World* m_world;
    b2Body* m_body;

    // Car dimensions (in pixels, world units)
    static constexpr f32 CAR_WIDTH  = 40.0f;
    static constexpr f32 CAR_LENGTH = 60.0f;
    static constexpr f32 WHEELBASE  = 40.0f;  // distance between axles

    // Physics tuning
    static constexpr f32 MAX_SPEED        = 30000.0f;   // Top speed cap
    static constexpr f32 DRIVE_FORCE      = 3000.0f;   // Dramatically increased to give actual top speed
    static constexpr f32 BRAKE_FORCE      = 3000.0f;   // Increased to allow braking at new speeds
    static constexpr f32 REVERSE_FORCE    = 1000.0f;
    static constexpr f32 STEER_SPEED      = 7.0f;      // rad/s (increased for sensitive steering)
    static constexpr f32 LATERAL_FRICTION = 16.0f;     // how quickly lateral velocity kills
    static constexpr f32 DRIFT_THRESHOLD  = 0.25f;     // slip angle to count as drifting

    f32 m_current_slip = 0.0f;

    // Box2D uses meters internally; we use 1:1 pixels
    static constexpr f32 PPM = 1.0f; // pixels per meter (we run Box2D in pixel-space)
};

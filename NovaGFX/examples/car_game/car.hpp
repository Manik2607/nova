#pragma once
#include <nova/nova.hpp>
#include <box2d/box2d.h>

extern const float PIXELS_PER_METER;

class Car {
public:
    Car(b2World* world, nova::Vector2f start_pos);

    void update(nova::f32 dt);
    void draw(nova::Renderer2D& renderer);

    nova::Vector2f get_position() const;
    bool is_part_of_car(b2Body* b) const {
        return b == m_chassis || b == m_rear_wheel || b == m_front_wheel;
    }

private:
    b2World* m_world;
    b2Body* m_chassis;
    b2Body* m_rear_wheel;
    b2Body* m_front_wheel;

    b2WheelJoint* m_rear_joint;
    b2WheelJoint* m_front_joint;

    // Tuning constants
    float m_motor_speed = 0.0f;
    const float MAX_MOTOR_SPEED = 50.0f; // Radians per second.
    const float MOTOR_ACCEL = 100.0f;
};

#pragma once
#include <nova/nova.hpp>
#include <box2d/box2d.h>

extern const float PIXELS_PER_METER;

class Car {
public:
    Car(b2World* world, nova::Vector2f start_pos, std::shared_ptr<nova::Texture2D> texture);

    void update(nova::f32 dt);
    void draw(nova::Renderer2D& renderer);
    void set_motor_torque(float torque);
    void set_suspension(float frequency_hz, float damping_ratio);

    nova::Vector2f get_position() const;
    nova::Vector2f get_rear_wheel_position() const;
    float get_speed() const;
    float get_angle() const;
    bool is_part_of_car(b2Body* b) const {
        return b == m_chassis || b == m_rear_wheel || b == m_front_wheel;
    }

    // Call this after changing any shape, wheel offset, or rest parameters
    void rebuild_physics();

    // Exposed tunable params
    float max_speed = 50.0f;      // rad/s (motor)
    float motor_accel = 100.0f;   // rad/s^2
    float motor_torque = 450.0f;

    float susp_hz = 10.0f;
    float susp_damp = 0.40f;
    float r_wheel_radius = 0.55f;
    float f_wheel_radius = 0.55f;

    // Wheel offsets
    float rear_wheel_x = -0.80f;
    float rear_wheel_y = 1.10f;
    float front_wheel_x = 1.20f;
    float front_wheel_y = 1.10f;

    // Spring Rest Offsets
    float r_spring_rest = 0.0f;
    float f_spring_rest = 0.0f;

    // Body bounds
    float body_min_x = -1.80f;
    float body_max_x = 1.90f;
    float body_min_y = 0.00f;
    float body_max_y = 1.00f;

    // Cabin bounds
    float cabin_min_x = -1.60f;
    float cabin_max_x = 0.70f;
    float cabin_min_y = -1.00f;
    float cabin_max_y = 0.00f;
    float cabin_roof_x = 0.20f;

    // Center of Mass
    float com_x = 0.2f;
    float com_y = 0.8f;

private:
    b2World* m_world;
    b2Body* m_chassis = nullptr;
    b2Body* m_rear_wheel = nullptr;
    b2Body* m_front_wheel = nullptr;

    b2WheelJoint* m_rear_joint = nullptr;
    b2WheelJoint* m_front_joint = nullptr;

    std::shared_ptr<nova::Texture2D> m_texture;
    float m_motor_speed = 0.0f;
};

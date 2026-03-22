#include "car.hpp"
#include <vector>

using namespace nova;

Car::Car(b2World* world, Vector2f start_pos) : m_world(world) {
    b2Vec2 pos(start_pos.x / PIXELS_PER_METER, start_pos.y / PIXELS_PER_METER);

    // 1. CHASSIS
    b2BodyDef chassisDef;
    chassisDef.type = b2_dynamicBody;
    chassisDef.position = pos;
    m_chassis = m_world->CreateBody(&chassisDef);

    b2PolygonShape chassisShape;
    // Main body: CCW winding order required by Box2D (in standard math space where Y is up)
    // For our Y-down screen space, we invert the Ys from the original and reverse order.
    b2Vec2 vertices[6];
    vertices[0].Set(-1.5f,  0.5f); // Bottom left
    vertices[1].Set(-1.5f, -0.2f); // Trunk
    vertices[2].Set(-1.1f, -0.9f); // Roof left
    vertices[3].Set( 0.0f, -0.9f); // Roof right
    vertices[4].Set( 1.5f,  0.0f); // Hood
    vertices[5].Set( 1.5f,  0.5f); // Bottom right
    chassisShape.Set(vertices, 6);
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &chassisShape;
    fixtureDef.density = 3.0f;
    fixtureDef.friction = 0.5f;
    fixtureDef.filter.groupIndex = -1; // Prevent wheels from colliding with chassis
    m_chassis->CreateFixture(&fixtureDef);

    // Lower the center of mass to prevent flipping
    b2MassData massData;
    m_chassis->GetMassData(&massData);
    massData.center.y += 0.8f; // Move it lower (positive Y is down in our screen space)
    m_chassis->SetMassData(&massData);

    // 2. WHEELS
    b2CircleShape wheelShape;
    wheelShape.m_radius = 0.75f; // Smaller monster truck tires

    b2FixtureDef wheelFixtureDef;
    wheelFixtureDef.shape = &wheelShape;
    wheelFixtureDef.density = 0.1f;
    wheelFixtureDef.friction = 0.9f; // High friction for driving
    wheelFixtureDef.restitution = 0.1f;
    wheelFixtureDef.filter.groupIndex = -1;

    b2BodyDef wheelDef;
    wheelDef.type = b2_dynamicBody;

    // Rear Wheel
    wheelDef.position = pos + b2Vec2(-1.4f, 1.1f);
    m_rear_wheel = m_world->CreateBody(&wheelDef);
    m_rear_wheel->CreateFixture(&wheelFixtureDef);

    // Front Wheel
    wheelDef.position = pos + b2Vec2(1.4f, 1.1f);
    m_front_wheel = m_world->CreateBody(&wheelDef);
    m_front_wheel->CreateFixture(&wheelFixtureDef);

    // 3. SUSPENSION JOINTS
    b2WheelJointDef jointDef;
    b2Vec2 axis(0.0f, 1.0f); // Suspension acts on the vertical axis

    // Rear suspension (driven)
    jointDef.Initialize(m_chassis, m_rear_wheel, m_rear_wheel->GetPosition(), axis);
    jointDef.motorSpeed = 0.0f;
    jointDef.maxMotorTorque = 600.0f; // Massive torque
    jointDef.enableMotor = true;
    jointDef.stiffness = 100.0f; // hz - extremely solid
    jointDef.damping = 0.5f;    // overdamped
    m_rear_joint = (b2WheelJoint*)m_world->CreateJoint(&jointDef);

    // Front suspension (driven - 2WD)
    jointDef.Initialize(m_chassis, m_front_wheel, m_front_wheel->GetPosition(), axis);
    jointDef.motorSpeed = 0.0f;
    jointDef.maxMotorTorque = 600.0f;
    jointDef.enableMotor = true;
    jointDef.stiffness = 100.0f;
    jointDef.damping = 0.5f;
    m_front_joint = (b2WheelJoint*)m_world->CreateJoint(&jointDef);
}

void Car::update(f32 dt) {
    (void)dt;
    bool has_input = false;

    if (Input::is_key_down(Key::D) || Input::is_key_down(Key::RIGHT)) {
        m_motor_speed += MOTOR_ACCEL * dt; // Forward is positive
        has_input = true;
    } else if (Input::is_key_down(Key::A) || Input::is_key_down(Key::LEFT)) {
        m_motor_speed -= MOTOR_ACCEL * dt; // Reverse is negative
        has_input = true;
    } else {
        // Natural friction/braking is removed so the car can coast/roll!
        // We sync the motor speed to the actual wheel joint speed so it resumes smoothly
        m_motor_speed = m_rear_wheel->GetAngularVelocity();
    }

    if (m_motor_speed > MAX_MOTOR_SPEED) m_motor_speed = MAX_MOTOR_SPEED;
    if (m_motor_speed < -MAX_MOTOR_SPEED) m_motor_speed = -MAX_MOTOR_SPEED;

    m_rear_joint->EnableMotor(has_input);
    m_front_joint->EnableMotor(has_input);

    if (has_input) {
        m_rear_joint->SetMotorSpeed(m_motor_speed);
        m_front_joint->SetMotorSpeed(m_motor_speed);
    }
}

void Car::draw(Renderer2D& renderer) {
    // Draw Chassis
    b2Vec2 pos = m_chassis->GetPosition();
    float angle = m_chassis->GetAngle();
    b2Fixture* fixture = m_chassis->GetFixtureList();
    b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();

    std::vector<Vector2f> vertices;
    for (int i = 0; i < poly->m_count; ++i) {
        b2Vec2 v = m_chassis->GetWorldPoint(poly->m_vertices[i]);
        vertices.push_back({v.x * PIXELS_PER_METER, v.y * PIXELS_PER_METER});
    }
    renderer.draw_polygon(vertices, Color(0.8f, 0.2f, 0.2f, 1.0f));
    renderer.draw_polygon_outline(vertices, Color::BLACK(), 3.0f);

    auto draw_wheel = [&](b2Body* wheel) {
        b2Vec2 wpos = wheel->GetPosition();
        float wangle = wheel->GetAngle();
        b2Fixture* wfixture = wheel->GetFixtureList();
        b2CircleShape* circle = (b2CircleShape*)wfixture->GetShape();
        
        Vector2f pcenter = {wpos.x * PIXELS_PER_METER, wpos.y * PIXELS_PER_METER};
        float radius = circle->m_radius * PIXELS_PER_METER;
        
        renderer.draw_circle(pcenter, radius, Color(0.2f, 0.2f, 0.2f, 1.0f), 24);
        renderer.draw_circle_outline(pcenter, radius, Color::BLACK(), 2.0f, 24);
        
        // Draw spoke
        Vector2f end_p = pcenter + Vector2f{std::cos(wangle) * radius, std::sin(wangle) * radius};
        renderer.draw_line(pcenter, end_p, Color::GRAY(), 3.0f);
    };

    draw_wheel(m_rear_wheel);
    draw_wheel(m_front_wheel);
}

Vector2f Car::get_position() const {
    b2Vec2 p = m_chassis->GetPosition();
    return {p.x * PIXELS_PER_METER, p.y * PIXELS_PER_METER};
}

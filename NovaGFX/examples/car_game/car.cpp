#include "car.hpp"
#include <vector>

using namespace nova;

Car::Car(b2World* world, Vector2f start_pos, std::shared_ptr<Texture2D> texture) 
    : m_world(world), m_texture(texture) {
    b2Vec2 pos(start_pos.x / PIXELS_PER_METER, start_pos.y / PIXELS_PER_METER);

    b2BodyDef chassisDef;
    chassisDef.type = b2_dynamicBody;
    chassisDef.position = pos;
    m_chassis = m_world->CreateBody(&chassisDef);

    rebuild_physics();
}

void Car::rebuild_physics() {
    b2Vec2 pos = m_chassis->GetPosition();
    float angle = m_chassis->GetAngle();
    b2Vec2 linear_vel = m_chassis->GetLinearVelocity();
    float angular_vel = m_chassis->GetAngularVelocity();

    // Destroy old joints and fixtures
    if (m_rear_joint) { m_world->DestroyJoint(m_rear_joint); m_rear_joint = nullptr; }
    if (m_front_joint) { m_world->DestroyJoint(m_front_joint); m_front_joint = nullptr; }

    if (m_rear_wheel) { m_world->DestroyBody(m_rear_wheel); m_rear_wheel = nullptr; }
    if (m_front_wheel) { m_world->DestroyBody(m_front_wheel); m_front_wheel = nullptr; }

    for (b2Fixture* f = m_chassis->GetFixtureList(); f; ) {
        b2Fixture* next = f->GetNext();
        m_chassis->DestroyFixture(f);
        f = next;
    }

    // Rebuild Chassis Fixtures
    b2PolygonShape bodyShape;
    b2Vec2 bodyVerts[4];
    bodyVerts[0].Set(body_min_x, body_max_y);
    bodyVerts[1].Set(body_max_x, body_max_y);
    bodyVerts[2].Set(body_max_x, body_min_y);
    bodyVerts[3].Set(body_min_x, body_min_y);
    bodyShape.Set(bodyVerts, 4);

    b2PolygonShape cabinShape;
    b2Vec2 cabinVerts[4];
    cabinVerts[0].Set(cabin_min_x, cabin_max_y);
    cabinVerts[1].Set(cabin_max_x, cabin_max_y);
    cabinVerts[2].Set(cabin_roof_x, cabin_min_y); // roof front
    cabinVerts[3].Set(cabin_min_x, cabin_min_y); // roof rear
    cabinShape.Set(cabinVerts, 4);
    
    b2FixtureDef fixtureDef;
    fixtureDef.density = 4.0f;
    fixtureDef.friction = 0.5f;
    fixtureDef.filter.groupIndex = -1;

    fixtureDef.shape = &bodyShape;
    m_chassis->CreateFixture(&fixtureDef);

    fixtureDef.shape = &cabinShape;
    m_chassis->CreateFixture(&fixtureDef);

    // Coordinate CoM
    b2MassData massData;
    m_chassis->GetMassData(&massData);
    massData.center.Set(com_x, com_y);
    m_chassis->SetMassData(&massData);

    // Rebuild Wheels
    b2CircleShape wheelShape;

    b2FixtureDef wheelFixtureDef;
    wheelFixtureDef.shape = &wheelShape;
    wheelFixtureDef.density = 0.3f;
    wheelFixtureDef.friction = 0.9f;
    wheelFixtureDef.restitution = 0.3f;
    wheelFixtureDef.filter.groupIndex = -1;

    b2BodyDef wheelDef;
    wheelDef.type = b2_dynamicBody;

    // Transform local wheel offsets to world
    b2Transform t;
    t.p = pos;
    t.q.Set(angle);

    wheelShape.m_radius = r_wheel_radius;
    wheelDef.position = b2Mul(t, b2Vec2(rear_wheel_x, rear_wheel_y));
    m_rear_wheel = m_world->CreateBody(&wheelDef);
    m_rear_wheel->CreateFixture(&wheelFixtureDef);
    m_rear_wheel->SetLinearVelocity(linear_vel);
    m_rear_wheel->SetAngularVelocity(angular_vel);

    wheelShape.m_radius = f_wheel_radius;
    wheelDef.position = b2Mul(t, b2Vec2(front_wheel_x, front_wheel_y));
    m_front_wheel = m_world->CreateBody(&wheelDef);
    m_front_wheel->CreateFixture(&wheelFixtureDef);
    m_front_wheel->SetLinearVelocity(linear_vel);
    m_front_wheel->SetAngularVelocity(angular_vel);

    // Rebuild Joints
    b2WheelJointDef jointDef;
    b2Vec2 world_axis = b2Mul(t.q, b2Vec2(0.0f, 1.0f));

    // Rear
    jointDef.Initialize(m_chassis, m_rear_wheel, m_rear_wheel->GetPosition(), world_axis);
    jointDef.localAnchorA.y -= r_spring_rest; // override anchor to add spring rest length
    
    jointDef.motorSpeed = 0.0f;
    jointDef.maxMotorTorque = motor_torque;
    jointDef.enableMotor = true;
    b2LinearStiffness(jointDef.stiffness, jointDef.damping, susp_hz, susp_damp, m_chassis, m_rear_wheel);
    m_rear_joint = (b2WheelJoint*)m_world->CreateJoint(&jointDef);

    // Front
    jointDef.Initialize(m_chassis, m_front_wheel, m_front_wheel->GetPosition(), world_axis);
    jointDef.localAnchorA.y -= f_spring_rest; 

    jointDef.motorSpeed = 0.0f;
    jointDef.maxMotorTorque = motor_torque;
    jointDef.enableMotor = true;
    b2LinearStiffness(jointDef.stiffness, jointDef.damping, susp_hz, susp_damp, m_chassis, m_front_wheel);
    m_front_joint = (b2WheelJoint*)m_world->CreateJoint(&jointDef);
}

void Car::update(f32 dt) {
    (void)dt;
    bool has_input = false;

    if (Input::is_key_down(Key::D) || Input::is_key_down(Key::RIGHT)) {
        m_motor_speed += motor_accel * dt;
        has_input = true;
    } else if (Input::is_key_down(Key::A) || Input::is_key_down(Key::LEFT)) {
        m_motor_speed -= motor_accel * dt;
        has_input = true;
    } else {
        m_motor_speed = m_rear_wheel->GetAngularVelocity();
    }

    if (m_motor_speed >  max_speed) m_motor_speed =  max_speed;
    if (m_motor_speed < -max_speed) m_motor_speed = -max_speed;

    m_rear_joint->EnableMotor(has_input);
    m_front_joint->EnableMotor(has_input);

    if (has_input) {
        m_rear_joint->SetMotorSpeed(m_motor_speed);
        m_front_joint->SetMotorSpeed(m_motor_speed);
    }
}

void Car::draw(Renderer2D& renderer) {
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

    // Draw Chassis Texture
    Vector2f pos = get_position();
    float angle = get_angle();
    
    // Jeep dimensions: roughly 4.2m x 2.1m
    Vector2f size = Vector2f(4.2f * PIXELS_PER_METER, 2.1f * PIXELS_PER_METER);
    
    // We want the center of the body (0, 0) to be the origin.
    if (m_texture) {
        // We use draw_sprite_region to flip the sprite horizontally
        // The original image faces right, but the collider was slightly mismatched.
        // If the user says it's facing wrong, we'll ensure it faces RIGHT by default with correct alignment.
        renderer.draw_sprite_region(*m_texture, 
            Rect2f({0, 0}, {(f32)m_texture->get_width(), (f32)m_texture->get_height()}), 
            pos, size, Color::WHITE(), angle);
    } else {
        // Fallback to debug DRAW if texture isn't loaded
        b2Fixture* f = m_chassis->GetFixtureList();
        while (f) {
            b2PolygonShape* poly = (b2PolygonShape*)f->GetShape();
            std::vector<Vector2f> vertices;
            for (int i = 0; i < poly->m_count; ++i) {
                b2Vec2 v = m_chassis->GetWorldPoint(poly->m_vertices[i]);
                vertices.push_back({v.x * PIXELS_PER_METER, v.y * PIXELS_PER_METER});
            }
            renderer.draw_polygon(vertices, Color(0.8f, 0.2f, 0.2f, 1.0f));
        }
    }

    // --- DEBUG COLLIDER RENDERING ---
    // Draw the physics shapes as outlines so we can see if they match the image
    b2Fixture* f = m_chassis->GetFixtureList();
    while (f) {
        if (f->GetShape()->GetType() == b2Shape::e_polygon) {
            b2PolygonShape* poly = (b2PolygonShape*)f->GetShape();
            std::vector<Vector2f> vertices;
            for (int i = 0; i < poly->m_count; ++i) {
                b2Vec2 v = m_chassis->GetWorldPoint(poly->m_vertices[i]);
                vertices.push_back({v.x * PIXELS_PER_METER, v.y * PIXELS_PER_METER});
            }
            renderer.draw_polygon_outline(vertices, Color::GREEN(), 2.0f);
        }
        f = f->GetNext();
    }

    // --- DEBUG COM RENDERING ---
    b2Vec2 world_com_b2 = m_chassis->GetWorldCenter();
    Vector2f world_com = {world_com_b2.x * PIXELS_PER_METER, world_com_b2.y * PIXELS_PER_METER};
    renderer.draw_circle(world_com, 6.0f, Color::RED(), 16);
    renderer.draw_circle_outline(world_com, 6.0f, Color::WHITE(), 2.0f, 16);
}


void Car::set_motor_torque(float torque) {
    motor_torque = torque;
    m_rear_joint->SetMaxMotorTorque(torque);
    m_front_joint->SetMaxMotorTorque(torque);
}

void Car::set_suspension(float frequency_hz, float damping_ratio) {
    float stiffness, damping;
    b2LinearStiffness(stiffness, damping, frequency_hz, damping_ratio, m_chassis, m_rear_wheel);
    m_rear_joint->SetStiffness(stiffness);
    m_rear_joint->SetDamping(damping);
    b2LinearStiffness(stiffness, damping, frequency_hz, damping_ratio, m_chassis, m_front_wheel);
    m_front_joint->SetStiffness(stiffness);
    m_front_joint->SetDamping(damping);
}

Vector2f Car::get_position() const {
    b2Vec2 p = m_chassis->GetPosition();
    return {p.x * PIXELS_PER_METER, p.y * PIXELS_PER_METER};
}

Vector2f Car::get_rear_wheel_position() const {
    b2Vec2 p = m_rear_wheel->GetPosition();
    return {p.x * PIXELS_PER_METER, p.y * PIXELS_PER_METER};
}

float Car::get_speed() const {
    return m_chassis->GetLinearVelocity().Length() * PIXELS_PER_METER;
}

float Car::get_angle() const {
    return m_chassis->GetAngle();
}

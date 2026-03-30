#include "car.hpp"
#include <nova/input/input.hpp>
#include <cmath>
#include <algorithm>
#include <array>

Car::Car(b2World* world, Vector2f start_pos, f32 start_angle)
    : m_world(world)
{
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.position.Set(start_pos.x, start_pos.y);
    bd.angle = start_angle;
    bd.linearDamping = 0.3f;
    bd.angularDamping = 5.0f;
    m_body = m_world->CreateBody(&bd);

    b2PolygonShape shape;
    shape.SetAsBox(CAR_WIDTH * 0.5f, CAR_LENGTH * 0.5f);

    b2FixtureDef fd;
    fd.shape = &shape;
    fd.density = 0.001f;  // very low — we override mass below
    fd.friction = 0.3f;
    fd.restitution = 0.3f;
    m_body->CreateFixture(&fd);

    // Override mass for predictable pixel-space physics
    b2MassData md;
    md.mass = 2.0f;  // 2 "kg" in pixel-space
    md.center = b2Vec2(0.0f, 0.0f);
    md.I = 2.0f * (CAR_WIDTH * CAR_WIDTH + CAR_LENGTH * CAR_LENGTH) / 12.0f;
    m_body->SetMassData(&md);
}

void Car::update(f32 dt, f32 surface_grip) {
    apply_steering(dt);
    apply_drive_force(dt);
    apply_friction(dt, surface_grip);

    // Calculate slip angle
    Vector2f vel = get_velocity();
    f32 speed = vel.length();
    if (speed > 10.0f) {
        Vector2f fwd = get_forward();
        Vector2f vel_dir = vel.normalized();
        m_current_slip = std::abs(fwd.cross(vel_dir));
    } else {
        m_current_slip = 0.0f;
    }
}

void Car::apply_drive_force(f32 /*dt*/) {
    f32 throttle = 0.0f;
    if (Input::is_key_down(Key::W) || Input::is_key_down(Key::UP))
        throttle = 1.0f;
    if (Input::is_key_down(Key::S) || Input::is_key_down(Key::DOWN))
        throttle = -1.0f;

    Vector2f fwd = get_forward();
    f32 speed = get_speed();

    f32 force = 0.0f;
    if (throttle > 0.0f) {
        // Apply full drive force regardless of current speed
        force = DRIVE_FORCE * throttle;
    } else if (throttle < 0.0f) {
        // Check if moving forward → brake, else reverse
        Vector2f vel = get_velocity();
        f32 forward_speed = fwd.dot(vel);
        if (forward_speed > 30.0f) {
            force = -BRAKE_FORCE; // braking
        } else {
            force = -REVERSE_FORCE; // reversing
        }
    }

    if (force != 0.0f) {
        b2Vec2 f_vec(fwd.x * force, fwd.y * force);
        m_body->ApplyForceToCenter(f_vec, true);
    }
}

void Car::apply_steering(f32 dt) {
    f32 steer_input = 0.0f;
    if (Input::is_key_down(Key::A) || Input::is_key_down(Key::LEFT))
        steer_input = -1.0f;
    if (Input::is_key_down(Key::D) || Input::is_key_down(Key::RIGHT))
        steer_input = 1.0f;

    // Do not steer if the car is basically stopped
    if (get_speed() < 10.0f) {
        return;
    }

    // Invert the steering direction if we are driving backwards
    Vector2f vel = get_velocity();
    Vector2f fwd = get_forward();
    if (fwd.dot(vel) < 0.0f) {
        steer_input = -steer_input;
    }

    // Directly set angular velocity. ultra simple and always works.
    if (steer_input != 0.0f) {
        m_body->SetAngularVelocity(steer_input * STEER_SPEED);
    }
}

void Car::apply_friction(f32 dt, f32 surface_grip) {
    // Kill lateral velocity (this creates the drift feel)
    Vector2f right = get_right();
    Vector2f vel = get_velocity();

    f32 lateral_speed = right.dot(vel);

    // Lateral friction: reduce lateral velocity
    // Lower grip = less lateral friction = more slide
    f32 friction = LATERAL_FRICTION * surface_grip;

    // When drifting, reduce friction further to maintain the slide
    if (std::abs(lateral_speed) > 30.0f) {
        friction *= 0.5f; // drift maintains momentum
    }

    // Apply lateral friction as impulse (counteracts sideways slide)
    f32 impulse_mag = -lateral_speed * friction * m_body->GetMass() * dt;
    b2Vec2 impulse(right.x * impulse_mag, right.y * impulse_mag);
    m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);
}

Vector2f Car::get_position() const {
    b2Vec2 p = m_body->GetPosition();
    return {p.x, p.y};
}

Vector2f Car::get_velocity() const {
    b2Vec2 v = m_body->GetLinearVelocity();
    return {v.x, v.y};
}

f32 Car::get_angle() const {
    return m_body->GetAngle();
}

f32 Car::get_speed() const {
    return get_velocity().length();
}

f32 Car::get_slip_angle() const {
    return m_current_slip;
}

bool Car::is_drifting() const {
    return m_current_slip > DRIFT_THRESHOLD && get_speed() > 30.0f;
}

Vector2f Car::get_forward() const {
    f32 a = m_body->GetAngle();
    return {-std::sin(a), std::cos(a)};
}

Vector2f Car::get_right() const {
    f32 a = m_body->GetAngle();
    return {std::cos(a), std::sin(a)};
}

void Car::get_rear_wheel_positions(Vector2f& left, Vector2f& right_out) const {
    Vector2f pos = get_position();
    Vector2f fwd = get_forward();
    Vector2f rgt = get_right();
    Vector2f rear_center = pos - fwd * (WHEELBASE * 0.4f);
    left = rear_center - rgt * (CAR_WIDTH * 0.4f);
    right_out = rear_center + rgt * (CAR_WIDTH * 0.4f);
}

void Car::draw(Renderer2D& renderer) {
    Vector2f pos = get_position();
    Vector2f fwd = get_forward();
    Vector2f rgt = get_right();

    // Color scheme
    Color body_main = is_drifting()
        ? Color(1.0f, 0.15f, 0.5f, 1.0f)    // hot pink
        : Color(0.05f, 0.7f, 0.95f, 1.0f);   // electric cyan
    Color body_dark = is_drifting()
        ? Color(0.6f, 0.05f, 0.3f, 1.0f)
        : Color(0.02f, 0.35f, 0.6f, 1.0f);
    Color accent = Color(1.0f, 0.85f, 0.1f, 1.0f); // gold accent

    f32 hw = CAR_WIDTH * 0.5f;
    f32 hl = CAR_LENGTH * 0.5f;

    // Helper: rotate point around car center
    auto xform = [&](f32 rx, f32 ry) -> Vector2f {
        return pos + fwd * ry + rgt * rx;
    };

    // ── Underglow (neon ground halo) ──
    {
        Color glow = body_main;
        glow.a = 0.12f;
        std::array<Vector2f, 8> glow_pts = {
            xform(-hw - 6, -hl - 4), xform(hw + 6, -hl - 4),
            xform(hw + 8,  -hl * 0.3f), xform(hw + 8,  hl * 0.3f),
            xform(hw + 6,  hl + 4),  xform(-hw - 6, hl + 4),
            xform(-hw - 8, hl * 0.3f), xform(-hw - 8, -hl * 0.3f)
        };
        renderer.draw_polygon(glow_pts, glow);
    }

    // ── Main body (low-poly angular shape) ──
    {
        // Tapered nose, wide middle, narrower tail
        std::array<Vector2f, 8> body = {
            xform(-hw * 0.5f, -hl),        // front-left (narrow nose)
            xform( hw * 0.5f, -hl),         // front-right
            xform( hw,        -hl * 0.4f),  // front-shoulder right
            xform( hw,         hl * 0.5f),  // rear-shoulder right
            xform( hw * 0.85f, hl),         // rear-right
            xform(-hw * 0.85f, hl),         // rear-left
            xform(-hw,         hl * 0.5f),  // rear-shoulder left
            xform(-hw,        -hl * 0.4f),  // front-shoulder left
        };
        renderer.draw_polygon(body, body_main);

        // Dark center stripe
        std::array<Vector2f, 4> stripe = {
            xform(-hw * 0.15f, -hl * 0.8f),
            xform( hw * 0.15f, -hl * 0.8f),
            xform( hw * 0.15f,  hl * 0.85f),
            xform(-hw * 0.15f,  hl * 0.85f),
        };
        renderer.draw_polygon(stripe, body_dark);
    }

    // ── Windshield ──
    {
        Color windshield(0.15f, 0.2f, 0.4f, 0.85f);
        std::array<Vector2f, 4> ws = {
            xform(-hw * 0.4f, -hl * 0.5f),
            xform( hw * 0.4f, -hl * 0.5f),
            xform( hw * 0.55f, -hl * 0.15f),
            xform(-hw * 0.55f, -hl * 0.15f),
        };
        renderer.draw_polygon(ws, windshield);
    }

    // ── Side panels (accent color) ──
    {
        // Left panel
        std::array<Vector2f, 4> left_p = {
            xform(-hw,         -hl * 0.3f),
            xform(-hw * 0.75f, -hl * 0.3f),
            xform(-hw * 0.75f,  hl * 0.35f),
            xform(-hw,          hl * 0.35f),
        };
        renderer.draw_polygon(left_p, accent);
        // Right panel
        std::array<Vector2f, 4> right_p = {
            xform(hw * 0.75f, -hl * 0.3f),
            xform(hw,         -hl * 0.3f),
            xform(hw,          hl * 0.35f),
            xform(hw * 0.75f,  hl * 0.35f),
        };
        renderer.draw_polygon(right_p, accent);
    }

    // ── Headlights ──
    {
        Color headlight(1.0f, 1.0f, 0.95f, 1.0f);
        Color headlight_glow(1.0f, 1.0f, 0.8f, 0.25f);
        // Left headlight
        std::array<Vector2f, 3> hl_l = {
            xform(-hw * 0.45f, -hl),
            xform(-hw * 0.15f, -hl),
            xform(-hw * 0.3f,  -hl * 0.85f),
        };
        renderer.draw_polygon(hl_l, headlight);
        renderer.draw_circle(xform(-hw * 0.3f, -hl * 0.95f), 6.0f, headlight_glow);
        // Right headlight
        std::array<Vector2f, 3> hl_r = {
            xform(hw * 0.15f, -hl),
            xform(hw * 0.45f, -hl),
            xform(hw * 0.3f,  -hl * 0.85f),
        };
        renderer.draw_polygon(hl_r, headlight);
        renderer.draw_circle(xform(hw * 0.3f, -hl * 0.95f), 6.0f, headlight_glow);
    }

    // ── Taillights ──
    {
        Color tail(1.0f, 0.05f, 0.15f, 1.0f);
        Color tail_glow = tail;
        tail_glow.a = 0.3f;
        // Left taillight
        std::array<Vector2f, 4> tl_l = {
            xform(-hw * 0.8f,  hl * 0.85f),
            xform(-hw * 0.3f,  hl * 0.85f),
            xform(-hw * 0.3f,  hl),
            xform(-hw * 0.8f,  hl),
        };
        renderer.draw_polygon(tl_l, tail);
        renderer.draw_circle(xform(-hw * 0.55f, hl * 0.92f), 5.0f, tail_glow);
        // Right taillight
        std::array<Vector2f, 4> tl_r = {
            xform(hw * 0.3f,  hl * 0.85f),
            xform(hw * 0.8f,  hl * 0.85f),
            xform(hw * 0.8f,  hl),
            xform(hw * 0.3f,  hl),
        };
        renderer.draw_polygon(tl_r, tail);
        renderer.draw_circle(xform(hw * 0.55f, hl * 0.92f), 5.0f, tail_glow);
    }

    // ── Body outline (neon edge glow) ──
    {
        Color outline = body_main;
        outline.a = 0.6f;
        std::array<Vector2f, 8> body_outline = {
            xform(-hw * 0.5f, -hl),
            xform( hw * 0.5f, -hl),
            xform( hw,        -hl * 0.4f),
            xform( hw,         hl * 0.5f),
            xform( hw * 0.85f, hl),
            xform(-hw * 0.85f, hl),
            xform(-hw,         hl * 0.5f),
            xform(-hw,        -hl * 0.4f),
        };
        renderer.draw_polygon_outline(body_outline, outline, 2.0f);
    }
}

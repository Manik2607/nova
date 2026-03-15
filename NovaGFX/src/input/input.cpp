#include "nova/input/input.hpp"
#include <GLFW/glfw3.h>

namespace nova {

std::bitset<Input::MAX_KEYS> Input::s_keys_current;
std::bitset<Input::MAX_KEYS> Input::s_keys_previous;

std::bitset<Input::MAX_MOUSE_BUTTONS> Input::s_mouse_current;
std::bitset<Input::MAX_MOUSE_BUTTONS> Input::s_mouse_previous;

Vector2f Input::s_mouse_pos;
Vector2f Input::s_mouse_delta;
Vector2f Input::s_scroll_delta;
Vector2f Input::s_scroll_accum;

Input::GamepadState Input::s_gamepads[4];

void Input::update() {
    s_keys_previous = s_keys_current;
    s_mouse_previous = s_mouse_current;

    static Vector2f s_last_frame_pos = s_mouse_pos;
    s_mouse_delta = s_mouse_pos - s_last_frame_pos;
    s_last_frame_pos = s_mouse_pos;

    s_scroll_delta = s_scroll_accum;
    s_scroll_accum = {0.0f, 0.0f};

    for (int i = 0; i < 4; ++i) {
        if (glfwJoystickPresent(GLFW_JOYSTICK_1 + i) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + i)) {
            s_gamepads[i].connected = true;
            s_gamepads[i].buttons_previous = s_gamepads[i].buttons_current;
            
            GLFWgamepadstate state;
            if (glfwGetGamepadState(GLFW_JOYSTICK_1 + i, &state)) {
                for (int a = 0; a < static_cast<int>(GamepadAxis::MAX_AXES); ++a) {
                    s_gamepads[i].axes[a] = state.axes[a];
                }
                for (int b = 0; b < static_cast<int>(GamepadButton::MAX_BUTTONS); ++b) {
                    s_gamepads[i].buttons_current[b] = (state.buttons[b] == GLFW_PRESS);
                }
            }
        } else {
            s_gamepads[i].connected = false;
        }
    }
}

bool Input::is_key_down(Key key) {
    auto k = static_cast<usize>(key);
    if (k >= MAX_KEYS) return false;
    return s_keys_current[k];
}

bool Input::is_key_pressed(Key key) {
    auto k = static_cast<usize>(key);
    if (k >= MAX_KEYS) return false;
    return s_keys_current[k] && !s_keys_previous[k];
}

bool Input::is_key_released(Key key) {
    auto k = static_cast<usize>(key);
    if (k >= MAX_KEYS) return false;
    return !s_keys_current[k] && s_keys_previous[k];
}

Vector2f Input::get_mouse_position() { return s_mouse_pos; }
Vector2f Input::get_mouse_delta() { return s_mouse_delta; }
Vector2f Input::get_scroll_delta() { return s_scroll_delta; }

bool Input::is_mouse_down(MouseButton button) {
    auto b = static_cast<usize>(button);
    if (b >= MAX_MOUSE_BUTTONS) return false;
    return s_mouse_current[b];
}

bool Input::is_mouse_pressed(MouseButton button) {
    auto b = static_cast<usize>(button);
    if (b >= MAX_MOUSE_BUTTONS) return false;
    return s_mouse_current[b] && !s_mouse_previous[b];
}

bool Input::is_mouse_released(MouseButton button) {
    auto b = static_cast<usize>(button);
    if (b >= MAX_MOUSE_BUTTONS) return false;
    return !s_mouse_current[b] && s_mouse_previous[b];
}

bool Input::is_gamepad_connected(i32 id) {
    if (id < 0 || id >= 4) return false;
    return s_gamepads[id].connected;
}

f32 Input::get_gamepad_axis(i32 id, GamepadAxis axis) {
    if (id < 0 || id >= 4) return 0.0f;
    return s_gamepads[id].axes[static_cast<usize>(axis)];
}

bool Input::is_gamepad_button_down(i32 id, GamepadButton button) {
    if (id < 0 || id >= 4) return false;
    return s_gamepads[id].buttons_current[static_cast<usize>(button)];
}

bool Input::is_gamepad_button_pressed(i32 id, GamepadButton button) {
    if (id < 0 || id >= 4) return false;
    auto b = static_cast<usize>(button);
    return s_gamepads[id].buttons_current[b] && !s_gamepads[id].buttons_previous[b];
}

void Input::internal_key_callback(int key, int action) {
    if (key >= 0 && key < static_cast<int>(MAX_KEYS)) {
        if (action == GLFW_PRESS) s_keys_current[key] = true;
        else if (action == GLFW_RELEASE) s_keys_current[key] = false;
    }
}

void Input::internal_mouse_button_callback(int button, int action) {
    if (button >= 0 && button < static_cast<int>(MAX_MOUSE_BUTTONS)) {
        if (action == GLFW_PRESS) s_mouse_current[button] = true;
        else if (action == GLFW_RELEASE) s_mouse_current[button] = false;
    }
}

void Input::internal_cursor_pos_callback(f32 x, f32 y) {
    s_mouse_pos = {x, y};
}

void Input::internal_scroll_callback(f32 x, f32 y) {
    s_scroll_accum += Vector2f(x, y);
}

void Input::internal_joystick_callback(int jid, int event) {
    (void)jid; (void)event;
}

} // namespace nova

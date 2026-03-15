#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/input/keyboard.hpp"
#include "nova/input/mouse.hpp"
#include "nova/input/gamepad.hpp"
#include <bitset>

namespace nova {

class Input {
public:
    static void update();

    static bool is_key_down(Key key);
    static bool is_key_pressed(Key key);
    static bool is_key_released(Key key);

    static Vector2f get_mouse_position();
    static Vector2f get_mouse_delta();
    static Vector2f get_scroll_delta();
    static bool is_mouse_down(MouseButton button);
    static bool is_mouse_pressed(MouseButton button);
    static bool is_mouse_released(MouseButton button);

    static bool is_gamepad_connected(i32 id);
    static f32 get_gamepad_axis(i32 id, GamepadAxis axis);
    static bool is_gamepad_button_down(i32 id, GamepadButton button);
    static bool is_gamepad_button_pressed(i32 id, GamepadButton button);

    static void internal_key_callback(int key, int action);
    static void internal_mouse_button_callback(int button, int action);
    static void internal_cursor_pos_callback(f32 x, f32 y);
    static void internal_scroll_callback(f32 x, f32 y);
    static void internal_joystick_callback(int jid, int event);

private:
    static constexpr usize MAX_KEYS = static_cast<usize>(Key::MAX_KEYS);
    static constexpr usize MAX_MOUSE_BUTTONS = static_cast<usize>(MouseButton::MAX_BUTTONS);

    static std::bitset<MAX_KEYS> s_keys_current;
    static std::bitset<MAX_KEYS> s_keys_previous;

    static std::bitset<MAX_MOUSE_BUTTONS> s_mouse_current;
    static std::bitset<MAX_MOUSE_BUTTONS> s_mouse_previous;

    static Vector2f s_mouse_pos;
    static Vector2f s_mouse_delta;
    static Vector2f s_scroll_delta;
    static Vector2f s_scroll_accum;

    struct GamepadState {
        bool connected = false;
        f32 axes[static_cast<usize>(GamepadAxis::MAX_AXES)]{0};
        std::bitset<static_cast<usize>(GamepadButton::MAX_BUTTONS)> buttons_current;
        std::bitset<static_cast<usize>(GamepadButton::MAX_BUTTONS)> buttons_previous;
    };
    static GamepadState s_gamepads[4];
};

} // namespace nova

#pragma once
#include "nova/core.hpp"

namespace nova {

enum class GamepadAxis {
    LEFT_X = 0,
    LEFT_Y = 1,
    RIGHT_X = 2,
    RIGHT_Y = 3,
    LEFT_TRIGGER = 4,
    RIGHT_TRIGGER = 5,
    MAX_AXES = 6
};

enum class GamepadButton {
    A = 0, B = 1, X = 2, Y = 3,
    LEFT_BUMPER = 4, RIGHT_BUMPER = 5,
    BACK = 6, START = 7, GUIDE = 8,
    LEFT_THUMB = 9, RIGHT_THUMB = 10,
    DPAD_UP = 11, DPAD_RIGHT = 12, DPAD_DOWN = 13, DPAD_LEFT = 14,
    MAX_BUTTONS = 15
};

} // namespace nova

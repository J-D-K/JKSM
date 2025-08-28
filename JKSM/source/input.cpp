#include "input.hpp"

#include <3ds.h>

namespace
{
    uint32_t s_buttonsDown{};
    uint32_t s_buttonsHeld{};
    uint32_t s_buttonsReleased{};
}

void input::update()
{
    hidScanInput();

    s_buttonsDown     = hidKeysDown();
    s_buttonsHeld     = hidKeysHeld();
    s_buttonsReleased = hidKeysUp();
}

bool input::button_pressed(uint32_t button) { return s_buttonsDown & button; }

bool input::button_held(uint32_t button) { return s_buttonsHeld & button; }

bool input::button_released(uint32_t button) { return s_buttonsReleased & button; }

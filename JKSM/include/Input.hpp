#pragma once
#include <cstdint>

namespace Input
{
    void Update();

    bool ButtonPressed(uint32_t Button);
    bool ButtonHeld(uint32_t Button);
    bool ButtonReleased(uint32_t Button);
} // namespace Input

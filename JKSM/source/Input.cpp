#include "Input.hpp"
#include <3ds.h>

namespace
{
    uint32_t s_PreviousFrame = 0;
    uint32_t s_CurrentFrame = 0;
} // namespace

void Input::Update(void)
{
    // Scan input
    hidScanInput();

    // Set previous
    s_PreviousFrame = s_CurrentFrame;
    // Update current
    s_CurrentFrame = hidKeysDown();
}

bool Input::ButtonPressed(uint32_t Button)
{
    return (s_CurrentFrame & Button) && !(s_PreviousFrame & Button);
}

bool Input::ButtonHeld(uint32_t Button)
{
    return (s_CurrentFrame & Button) && (s_PreviousFrame & Button);
}

bool Input::ButtonReleased(uint32_t Button)
{
    return !(s_CurrentFrame & Button) && (s_PreviousFrame & Button);
}

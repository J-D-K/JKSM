#include "Input.hpp"

#include <3ds.h>

void Input::Update()
{
    // Scan input
    hidScanInput();
}

bool Input::ButtonPressed(uint32_t Button) { return hidKeysDown() & Button; }

bool Input::ButtonHeld(uint32_t Button) { return hidKeysHeld() & Button; }

bool Input::ButtonReleased(uint32_t Button) { return hidKeysUp() & Button; }

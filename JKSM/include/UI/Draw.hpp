#pragma once
#include "SDL/SDL.hpp"

namespace UI
{
    // Draws a dialog box at X, Y with Width, Height.
    void DrawDialogBox(SDL_Surface *Target, int X, int Y, int Width, int Height);
} // namespace UI

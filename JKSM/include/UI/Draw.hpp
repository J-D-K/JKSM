#pragma once
#include "SDL/SDL.hpp"

namespace UI
{
    // Draws a dialog box at X, Y with Width, Height.
    void DrawDialogBox(SDL_Surface *Target, int X, int Y, int Width, int Height);
    // Draws a selection bounding box at X, Y with Width, Height. Color mod is the pulsing.
    void DrawBoundingBox(SDL_Surface *Target, int X, int Y, int Width, int Height, uint8_t ColorMod);
} // namespace UI

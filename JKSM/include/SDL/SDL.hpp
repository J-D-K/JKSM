#pragma once
#include "SDL/Color.hpp"
#include "SDL/Font.hpp"
#include "SDL/ResourceManager.hpp"
#include "SDL/Surface.hpp"
#include "logging/logger.hpp"

#include <SDL/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace SDL
{
    // Inits 3DS SDL for video, timers and maybe sound later.
    bool Initialize();
    // Closes 3DS SDL
    void Exit();
    // Begins a frame and clears screen surfaces. Sets the buffer to point to the top screen.
    void FrameBegin();
    // Ends blitting and drawing on the top screen and changes to bottom.
    void FrameChangeScreens();
    // Ends the current frame and flips it to the screen.
    void FrameEnd();
    // Returns the current screen buffer.
    SDL_Surface *GetCurrentBuffer();
    // Draws a rect to target at X, Y with Width, Height with Color
    void DrawRect(SDL_Surface *Target, int X, int Y, int Width, int Height, SDL::Color Color);
} // namespace SDL

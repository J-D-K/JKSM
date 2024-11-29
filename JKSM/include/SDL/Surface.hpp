#pragma once
#include "SDL/Color.hpp"
#include <SDL/SDL.h>
#include <string_view>

namespace SDL
{
    class Surface
    {
        public:
            Surface(void) = default;
            // Creates and empty surface Width and Height in size. AlphaBlended with whether or not the surface should be blended when blitted.
            Surface(int Width, int Height, bool AlphaBlended = true);
            // Loads image from file path
            Surface(std::string_view FilePath, bool AlphaBlended = true);
            // Makes internal m_Surface pointer point to surface.
            Surface(SDL_Surface *ExternalSurface, bool AlphaBlended = true);
            // Calls SDL_FreeSurface at destruction
            ~Surface();
            // Returns pointer to internal SDL_Surface.
            SDL_Surface *Get(void);
            // Blits surface at X, Y.
            void BlitAt(SDL_Surface *Target, int X, int Y);
            // Blits part of the surface at X, Y.
            void BlitPartAt(SDL_Surface *Target, int X, int Y, int SourceX, int SourceY, int SourceWidth, int SourceHeight);
            // This is kind of dangerous and is only used for one thing in JKSM that is one color.
            void ChangePixelsToColor(SDL::Color Color);

        private:
            // Underlying SDL_Surface
            SDL_Surface *m_Surface = nullptr;
            // Disables alpha blending for surface.
            void DisableAlphaBlending(void);
    };
} // namespace SDL

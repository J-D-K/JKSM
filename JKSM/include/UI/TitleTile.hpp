#pragma once
#include "SDL/SDL.hpp"

namespace UI
{
    // This can't really be part of ui element since it needs a position to draw at.
    class TitleTile
    {
        public:
            TitleTile(bool IsFavorite, SDL::SharedSurface Icon);

            void DrawAt(SDL_Surface *Target, int X, int Y);

        private:
            bool m_IsFavorite = false;
            SDL::SharedSurface m_Icon = nullptr;
    };
} // namespace UI

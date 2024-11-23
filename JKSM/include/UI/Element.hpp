#pragma once
#include "SDL/SDL.hpp"

namespace UI
{
    class Element
    {
        public:
            Element(void) = default;
            virtual ~Element() {};

            virtual void Update(void) = 0;
            virtual void Draw(SDL_Surface *Target) = 0;
    };
} // namespace UI

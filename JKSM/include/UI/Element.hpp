#pragma once
#include "SDL/SDL.hpp"

namespace UI
{
    class Element
    {
        public:
            Element() = default;
            virtual ~Element() {};

            virtual void Update()                  = 0;
            virtual void Draw(SDL_Surface *Target) = 0;
    };
} // namespace UI

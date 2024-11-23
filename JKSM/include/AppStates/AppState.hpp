#pragma once
#include "SDL/SDL.hpp"

class AppState
{
    public:
        AppState(void) = default;
        virtual ~AppState() {};

        virtual void Update(void) = 0;
        virtual void DrawTop(SDL_Surface *Target) = 0;
        virtual void DrawBottom(SDL_Surface *Target) = 0;

        // I usually avoid having this stuff in headers now, but here I don't care.
        bool IsActive(void) const
        {
            return m_IsActive;
        }

        bool HasFocus(void) const
        {
            return m_HasFocus;
        }

        void Deactivate(void)
        {
            m_IsActive = false;
        }

        void TakeFocus(void)
        {
            m_HasFocus = false;
        }

        void GiveFocus(void)
        {
            m_HasFocus = true;
        }

    private:
        // Whether state is active or can be purged.
        bool m_IsActive = true;
        // Whether a state is at the back of the vector and has focus.
        bool m_HasFocus = false;
};

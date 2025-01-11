#pragma once
#include "AppStates/AppState.hpp"
#include <string>

class MessageState : public AppState
{
    public:
        MessageState(AppState *CreatingState, std::string_view MessageString);
        ~MessageState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // So we can draw the previous state underneath.
        AppState *m_CreatingState = nullptr;
        // Message displayed.
        std::string m_MessageString;
        // X coordinate of OK text.
        int m_OKX = 0;
};

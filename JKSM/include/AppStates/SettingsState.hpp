#pragma once
#include "AppStates/AppState.hpp"
#include "SDL/SDL.hpp"
#include "UI/Menu.hpp"

class SettingsState : public AppState
{
    public:
        SettingsState(void);
        ~SettingsState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // Coordinates to center text
        int m_SettingsX = 0;
        int m_DescriptionX = 0;
        // Menu containing settings
        std::unique_ptr<UI::Menu> m_SettingsMenu;
        // Noto to draw text with
        SDL::SharedFont m_Noto;
};

#pragma once
#include "AppStates/AppState.hpp"
#include "Data/Data.hpp"
#include "UI/Menu.hpp"

class TitleOptionState : public AppState
{
    public:
        TitleOptionState(AppState *CreatingState, const Data::TitleData *TargetTitle, Data::SaveDataType SaveType);
        ~TitleOptionState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // Pointer to creating state.
        AppState *m_CreatingState = nullptr;
        // Pointer to target title.
        const Data::TitleData *m_TargetTitle = nullptr;
        // Save data type we're working with
        Data::SaveDataType m_SaveType;
        // Menu
        UI::Menu m_OptionsMenu;
};

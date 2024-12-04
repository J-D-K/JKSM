#pragma once
#include "AppStates/BaseSelectionState.hpp"
#include "Data/Data.hpp"
#include "SDL/SDL.hpp"
#include "UI/TitleView.hpp"
#include <memory>

class TitleSelectionState : public BaseSelectionState
{
    public:
        TitleSelectionState(Data::SaveDataType SaveType);
        ~TitleSelectionState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

        // Refreshes title view.
        void Refresh(void);

    private:
        UI::TitleView m_TitleView;
        // X coordinate for bottom text.
        int m_TextX = 0;
};

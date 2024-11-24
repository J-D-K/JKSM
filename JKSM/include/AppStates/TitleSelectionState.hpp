#pragma once
#include "AppStates/AppState.hpp"
#include "Data/Data.hpp"
#include "SDL/SDL.hpp"
#include "UI/TitleView.hpp"
#include <memory>

class TitleSelectionState : public AppState
{
    public:
        TitleSelectionState(Data::SaveDataType SaveType);
        ~TitleSelectionState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // To do: Maybe not a pointer...
        std::unique_ptr<UI::TitleView> m_TitleView;
        // Noto for text
        SDL::SharedFont m_Noto;
        // X coordinate for bottom text.
        int m_TextX = 0;
        // SaveType used.
        Data::SaveDataType m_SaveType;
};

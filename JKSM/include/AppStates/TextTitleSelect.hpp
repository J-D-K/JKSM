#pragma once
#include "AppStates/BaseSelectionState.hpp"
#include "Data/Data.hpp"
#include "SDL/SDL.hpp"
#include "UI/Menu.hpp"
#include <memory>
#include <vector>

class TextTitleSelect : public BaseSelectionState
{
    public:
        TextTitleSelect(Data::SaveDataType SaveType);
        ~TextTitleSelect() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

        void Refresh(void);

    private:
        // Menu with game titles
        UI::Menu m_TitleMenu;
        // Vector of pointers to title data.
        std::vector<Data::TitleData *> m_TitleData;
        // Coordinate to center text.
        int m_TextX = 0;
};

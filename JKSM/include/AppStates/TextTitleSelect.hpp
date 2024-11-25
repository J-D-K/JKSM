#pragma once
#include "AppStates/AppState.hpp"
#include "Data/Data.hpp"
#include "SDL/SDL.hpp"
#include "UI/Menu.hpp"
#include <memory>
#include <vector>

class TextTitleSelect : public AppState
{
    public:
        TextTitleSelect(Data::SaveDataType SaveType);
        ~TextTitleSelect() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // Menu with game titles
        std::unique_ptr<UI::Menu> m_TitleMenu;
        // Save type
        Data::SaveDataType m_SaveType;
        // Vector of pointers to title data.
        std::vector<Data::TitleData *> m_TitleData;
        // Font to draw text
        SDL::SharedFont m_Noto = nullptr;
        // Coordinate to center text.
        int m_TextX = 0;
};

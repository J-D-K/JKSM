#pragma once
#include "Data/Data.hpp"
#include "SDL/SDL.hpp"
#include "UI/Menu.hpp"
#include "appstates/BaseSelectionState.hpp"

#include <memory>
#include <vector>

class TextTitleSelect final : public BaseSelectionState
{
    public:
        TextTitleSelect(Data::SaveDataType saveType);
        ~TextTitleSelect() {};

        void update() override;

        void draw_top(SDL_Surface *target) override;

        void draw_bottom(SDL_Surface *target) override;

        void refresh() override;

    private:
        /// @brief Menu with game titles
        UI::Menu m_titleMenu;

        /// @brief Vector of pointers to title data.
        std::vector<Data::TitleData *> m_titleData{};

        /// @brief Coordinate to center text.
        int m_textX{};
};

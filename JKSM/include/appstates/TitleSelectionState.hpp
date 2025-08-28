#pragma once
#include "Data/Data.hpp"
#include "SDL/SDL.hpp"
#include "UI/TitleView.hpp"
#include "appstates/BaseSelectionState.hpp"

#include <memory>

class TitleSelectionState : public BaseSelectionState
{
    public:
        TitleSelectionState(Data::SaveDataType saveType);

        ~TitleSelectionState() {};

        void update() override;

        void draw_top(SDL_Surface *target) override;

        void draw_bottom(SDL_Surface *target) override;

        // Refreshes title view.
        void refresh() override;

    private:
        UI::TitleView m_titleView;

        // X coordinate for bottom text.
        int m_textX{};
};

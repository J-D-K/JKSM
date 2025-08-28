#pragma once
#include "Data/Data.hpp"
#include "System/Task.hpp"
#include "UI/Menu.hpp"
#include "appstates/BaseState.hpp"

class TitleOptionState final : public BaseState
{
    public:
        TitleOptionState(BaseState *creatingState, const Data::TitleData *targetTitle, Data::SaveDataType saveType);

        ~TitleOptionState() {};

        void update() override;

        void draw_top(SDL_Surface *target) override;

        void draw_bottom(SDL_Surface *target) override;

    private:
        /// @brief Pointer to creating state.
        BaseState *m_creatingState{};

        /// @brief Pointer to target title.
        const Data::TitleData *m_targetTitle{};

        /// @brief Save data type we're working with
        Data::SaveDataType m_saveType{};

        // Menu
        UI::Menu m_optionsMenu;
};

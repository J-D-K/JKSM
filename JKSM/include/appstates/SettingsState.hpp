#pragma once
#include "SDL/SDL.hpp"
#include "UI/Menu.hpp"
#include "appstates/BaseState.hpp"

class SettingsState final : public BaseState
{
    public:
        SettingsState();

        ~SettingsState() {};

        /// @brief Update override.
        void update() override;

        /// @brief Draw top override.
        void draw_top(SDL_Surface *target);

        /// @brief Draw bottom override.
        void draw_bottom(SDL_Surface *target);

    private:
        /// @brief Coordinates to center text
        int m_settingsX{};
        int m_descriptionX{};

        /// @brief Menu containing settings
        UI::Menu m_settingsMenu;

        /// @brief This updates all of the needed settings menu strings.
        void update_menu_strings();

        /// @brief This is called when A is pressed and toggles or changes config options.
        void update_config();
};

#pragma once
#include "AppStates/AppState.hpp"
#include "SDL/SDL.hpp"
#include "UI/Element.hpp"
#include <string>
#include <vector>

namespace UI
{
    class Menu : public UI::Element
    {
        public:
            // Creates menu drawn at X and Y. MaxDrawLength is the maximum number of options to display before scrolling.
            Menu(int X, int Y, int MaxDrawLength);
            // Inits a menu from an array of string_views.
            Menu(int X, int Y, int MaxDrawLength, std::string_view *Options, size_t OptionCount);
            ~Menu()
            {
            }

            // Adds an option to the menu
            void AddOption(std::string_view Option);
            // Handles input
            void Update(void);
            // Draws menu to target.
            void Draw(SDL_Surface *Target);

        private:
            // Currently selected option
            int m_Selected = 0;
            // X and Y to draw to.
            int m_X = 0, m_Y = 0;
            // Maximum number of options to show at once.
            int m_OptionStart = 0, m_MaximumDrawLength = 0;
            // Vector of options.
            std::vector<std::string> m_Options;
            // Font
            SDL::SharedFont m_Noto = nullptr;
    };
} // namespace UI

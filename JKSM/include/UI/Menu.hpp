#pragma once
#include "AppStates/AppState.hpp"
#include "SDL/SDL.hpp"
#include "UI/Element.hpp"
#include <array>
#include <string>
#include <vector>

namespace UI
{
    class Menu : public UI::Element
    {
        public:
            // Creates menu drawn at X and Y. MaxDrawLength is the maximum number of options to display before scrolling.
            Menu(int X, int Y, int Width, int MaxDrawLength);
            ~Menu()
            {
            }

            // Adds an option to the menu
            void AddOption(std::string_view Option);
            // Edits the option at index.
            void EditOption(int Index, std::string_view Option);
            // Clears the option vector
            void Reset(void);
            // Handles input
            void Update(void);
            // Draws menu to target.
            void Draw(SDL_Surface *Target);
            // Returns the selected option.
            int GetSelected(void) const;
            // Returns the number of options
            size_t GetSize(void) const;

        private:
            // Currently selected option
            int m_Selected = 0;
            // X and Y to draw to.
            int m_X = 0, m_Y = 0;
            // Width of selection bounding thingy to draw
            int m_Width = 0;
            // Maximum number of options to show at once.
            int m_OptionStart = 0, m_MaximumDrawLength = 0;
            // Vector of options.
            std::vector<std::string> m_Options;
            // This keeps track of the length of the vector.
            int m_OptionsLength = -1;
            // Font
            SDL::SharedFont m_Noto = nullptr;
            // Color shift for bounding box.
            uint8_t m_ColorShift = 0;
            // Bool to control color shifting.
            bool m_ShiftDirection = true;
            // These methods aren't needed outside of the class and handle input
            void HandleUpPress(void);
            void HandleDownPress(void);
            void HandleLeftPress(void);
            void HandleRightPress(void);
    };
} // namespace UI

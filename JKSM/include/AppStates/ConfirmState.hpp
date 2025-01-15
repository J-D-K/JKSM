#pragma once
#include "AppStates/AppState.hpp"
#include "AppStates/ProgressTaskState.hpp"
#include "AppStates/TaskState.hpp"
#include "Input.hpp"
#include "JKSM.hpp"
#include "Strings.hpp"
#include "System/Task.hpp"
#include "UI/Draw.hpp"
#include <3ds.h>
#include <memory>
#include <string>

template <typename TaskType, typename StateType, typename StructType>
class ConfirmState : public AppState
{
    public:
        // Function type so we can store pointer to it.
        using ConfirmFunction = void (*)(TaskType *, std::shared_ptr<StructType>);
        // Constructor
        ConfirmState(AppState *CreatingState,
                     std::string_view QueryString,
                     bool HoldToConfirm,
                     ConfirmFunction OnConfirmation,
                     std::shared_ptr<StructType> DataStruct)
            : AppState(AppState::StateFlags::SemiLock), m_CreatingState(CreatingState), m_QueryString(QueryString), m_Hold(HoldToConfirm),
              m_YesString(Strings::GetStringByName(Strings::Names::YesNo, 0)), m_OnConfirmation(OnConfirmation), m_DataStruct(DataStruct)
        {
            m_YesString = Strings::GetStringByName(Strings::Names::YesNo, 0);
            m_YesX = 88 - (m_Noto->GetTextWidth(12, m_YesString.c_str()) / 2);
            m_NoX = 232 - (m_Noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::YesNo, 1)) / 2);
        }

        void Update(void)
        {
            if (Input::ButtonPressed(KEY_A) && !m_Hold)
            {
                AppState::Deactivate();
                JKSM::PushState(std::make_shared<StateType>(m_CreatingState, m_OnConfirmation, m_DataStruct));
            }
            else if (Input::ButtonPressed(KEY_A) && m_Hold && !m_APressed)
            {
                // Record starting ticks and set APressed.
                m_StartingTicks = SDL_GetTicks();
                m_APressed = true;

                // Set yes string to hold and realign it
                m_YesString = Strings::GetStringByName(Strings::Names::HoldingText, 0);
                m_YesX = 88 - (m_Noto->GetTextWidth(12, m_YesString.c_str()) / 2);
            }
            else if (Input::ButtonHeld(KEY_A) && m_Hold && m_APressed)
            {
                int CurrentTicks = SDL_GetTicks() - m_StartingTicks;

                // After three seconds, confirmed.
                if (CurrentTicks >= 3000)
                {
                    AppState::Deactivate();
                    JKSM::PushState(std::make_shared<StateType>(m_CreatingState, m_OnConfirmation, m_DataStruct));
                }
                else if (CurrentTicks >= 2000)
                {
                    m_YesString = Strings::GetStringByName(Strings::Names::HoldingText, 2);
                    // Realign Yes string
                    m_YesX = 88 - (m_Noto->GetTextWidth(12, m_YesString.c_str()) / 2);
                }
                else if (CurrentTicks >= 1000)
                {
                    m_YesString = Strings::GetStringByName(Strings::Names::HoldingText, 1);
                    // Realign Yes string
                    m_YesX = 88 - (m_Noto->GetTextWidth(12, m_YesString.c_str()) / 2);
                }
            }
            else if (Input::ButtonReleased(KEY_A) && m_Hold && m_APressed)
            {
                // Reset press
                m_APressed = false;

                // Reset YesString.
                m_YesString = Strings::GetStringByName(Strings::Names::YesNo, 0);
                m_YesX = 88 - (m_Noto->GetTextWidth(12, m_YesString.c_str()) / 2);
            }
            else if (Input::ButtonPressed(KEY_B))
            {
                AppState::Deactivate();
            }
        }

        void DrawTop(SDL_Surface *Target)
        {
            if (m_CreatingState)
            {
                m_CreatingState->DrawTop(Target);
            }
        }

        void DrawBottom(SDL_Surface *Target)
        {
            // Draw the underlying state first.
            if (m_CreatingState)
            {
                m_CreatingState->DrawBottom(Target);
            }

            // Draw the dialog box
            UI::DrawDialogBox(Target, 8, 18, 304, 204);
            // Render query string.
            m_Noto->BlitTextAt(Target, 30, 30, 12, 268, m_QueryString.c_str());
            // Fake buttons. Maybe real later.
            SDL::DrawRect(Target, 8, 190, 304, 1, SDL::Colors::White);
            SDL::DrawRect(Target, 160, 190, 1, 32, SDL::Colors::White);
            // Yes and no
            m_Noto->BlitTextAt(Target, m_YesX, 200, 12, m_Noto->NO_TEXT_WRAP, m_YesString.c_str());
            m_Noto->BlitTextAt(Target, m_NoX, 200, 12, m_Noto->NO_TEXT_WRAP, Strings::GetStringByName(Strings::Names::YesNo, 1));
        }

    private:
        // State that spawned the confirmation so we can draw it underneath.
        AppState *m_CreatingState = nullptr;
        // String with query to confirm.
        std::string m_QueryString;
        // Whether or not A should be held to confirm the action.
        bool m_Hold = false;
        // Whether or not A was pressed to prevent auto confirming.
        bool m_APressed = false;
        // String to display for yes/holding
        std::string m_YesString;
        // X coordinates for Yes and No.
        int m_YesX = 0, m_NoX = 0;
        // Starting tick count for when holding A is needed.
        int m_StartingTicks = 0;
        // Pointer to function to run on confirmation.
        ConfirmFunction m_OnConfirmation;
        // Shared pointer to data struct to pass on confirmation so no need to worry about memory leaks.
        std::shared_ptr<StructType> m_DataStruct;
};

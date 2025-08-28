#pragma once
#include "JKSM.hpp"
#include "Strings.hpp"
#include "System/Task.hpp"
#include "UI/Draw.hpp"
#include "appstates/BaseState.hpp"
#include "appstates/ProgressTaskState.hpp"
#include "appstates/TaskState.hpp"
#include "input.hpp"

#include <3ds.h>
#include <memory>
#include <string>

template <typename TaskType, typename StateType, typename StructType>
class ConfirmState final : public BaseState
{
    public:
        // Function type so we can store pointer to it.
        using ConfirmFunction = void (*)(TaskType *, std::shared_ptr<StructType>);
        // Constructor
        ConfirmState(BaseState *creatingState,
                     std::string_view queryString,
                     bool holdToConfirm,
                     ConfirmFunction onConfirmation,
                     std::shared_ptr<StructType> dataStruct)
            : BaseState(BaseState::StateFlags::SemiLock)
            , m_creatingState(creatingState)
            , m_queryString(queryString)
            , m_hold(holdToConfirm)
            , m_yesString(Strings::GetStringByName(Strings::Names::YesNo, 0))
            , m_onConfirmation(onConfirmation)
            , m_dataStruct(dataStruct)
        {
            m_yesX = 88 - (m_noto->GetTextWidth(12, m_yesString.c_str()) / 2);
            m_noX  = 232 - (m_noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::YesNo, 1)) / 2);
        }

        ~ConfirmState() {};

        void update() override
        {
            if (input::button_pressed(KEY_A) && !m_hold)
            {
                BaseState::deactivate();
                JKSM::PushState(std::make_shared<StateType>(m_creatingState, m_onConfirmation, m_dataStruct));
            }
            else if (input::button_pressed(KEY_A) && m_hold && !m_triggerGuard)
            {
                // Record starting ticks and set APressed.
                m_startingTicks = SDL_GetTicks();
                m_triggerGuard  = true;

                // Set yes string to hold and realign it
                m_yesString = Strings::GetStringByName(Strings::Names::HoldingText, 0);
                m_yesX      = 88 - (m_noto->GetTextWidth(12, m_yesString.c_str()) / 2);
            }
            else if (input::button_held(KEY_A) && m_hold && m_triggerGuard)
            {
                int CurrentTicks = SDL_GetTicks() - m_startingTicks;

                // After three seconds, confirmed.
                if (CurrentTicks >= 3000)
                {
                    BaseState::deactivate();
                    JKSM::PushState(std::make_shared<StateType>(m_creatingState, m_onConfirmation, m_dataStruct));
                }
                else if (CurrentTicks >= 2000)
                {
                    m_yesString = Strings::GetStringByName(Strings::Names::HoldingText, 2);
                    // Realign Yes string
                    m_yesX = 88 - (m_noto->GetTextWidth(12, m_yesString.c_str()) / 2);
                }
                else if (CurrentTicks >= 1000)
                {
                    m_yesString = Strings::GetStringByName(Strings::Names::HoldingText, 1);
                    // Realign Yes string
                    m_yesX = 88 - (m_noto->GetTextWidth(12, m_yesString.c_str()) / 2);
                }
            }
            else if (input::button_released(KEY_A) && m_hold && m_triggerGuard)
            {
                // Reset press
                m_triggerGuard = false;

                // Reset YesString.
                m_yesString = Strings::GetStringByName(Strings::Names::YesNo, 0);
                m_yesX      = 88 - (m_noto->GetTextWidth(12, m_yesString.c_str()) / 2);
            }
            else if (input::button_pressed(KEY_B)) { BaseState::deactivate(); }
        }

        void draw_top(SDL_Surface *target) override
        {
            if (m_creatingState) { m_creatingState->draw_top(target); }
        }

        void draw_bottom(SDL_Surface *target) override
        {
            // Draw the underlying state first.
            if (m_creatingState) { m_creatingState->draw_bottom(target); }

            // Draw the dialog box
            UI::DrawDialogBox(target, 8, 18, 304, 204);
            // Render query string.
            m_noto->BlitTextAt(target, 30, 30, 12, 268, m_queryString.c_str());
            // Fake buttons. Maybe real later.
            SDL::DrawRect(target, 8, 190, 304, 1, SDL::Colors::White);
            SDL::DrawRect(target, 160, 190, 1, 32, SDL::Colors::White);
            // Yes and no
            m_noto->BlitTextAt(target, m_yesX, 200, 12, m_noto->NO_TEXT_WRAP, m_yesString.c_str());
            m_noto
                ->BlitTextAt(target, m_noX, 200, 12, m_noto->NO_TEXT_WRAP, Strings::GetStringByName(Strings::Names::YesNo, 1));
        }

    private:
        /// @brief Pointer to the spawning state so we can render it underneath
        BaseState *m_creatingState{};

        /// @brief String to display.
        std::string m_queryString;

        /// @brief Whether or not holding A is required.
        bool m_hold{};

        /// @brief This is to guard against A auto triggering.
        bool m_triggerGuard{};

        /// @brief The Yes string to display.
        std::string m_yesString;

        ///@brief X coordinates for Yes and No.
        int m_yesX{}, m_noX{};

        // Starting tick count for when holding A is needed.
        int m_startingTicks{};

        /// @brief Pointer to function to run on confirmation.
        ConfirmFunction m_onConfirmation;

        /// @brief Shared pointer to data struct to pass on confirmation so no need to worry about memory leaks.
        std::shared_ptr<StructType> m_dataStruct;
};

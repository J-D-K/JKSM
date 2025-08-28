#pragma once
#include "Assets.hpp"
#include "SDL/SDL.hpp"

class BaseState
{
    public:
        // This is needed so JKSM knows whether or not to allow exiting or state changing.
        // Semi-lock allows closing JKSM, but not state changing.
        // Full lock means neither.
        enum class StateFlags
        {
            Normal,
            SemiLock,
            Lock
        };

        /// @brief Base state constructor.
        BaseState(StateFlags type = StateFlags::Normal);

        virtual ~BaseState() {};

        /// @brief Virtual function for update routines.
        virtual void update() = 0;

        /// @brief Virtual function for drawing the top screen.
        virtual void draw_top(SDL_Surface *target) = 0;

        /// @brief Virtual function for draw the bottom screen.
        virtual void draw_bottom(SDL_Surface *target) = 0;

        /// @brief Returns whether or not the state is active or can be purged.
        bool is_active() const;

        /// @brief Returns whether or not the state has focus.
        bool has_focus() const;

        /// @brief Deactivates the state and marks it for purging.
        void deactivate();

        /// @brief Takes focus away from the state.
        void take_focus();

        /// @brief Gives focus to the state.
        void give_focus();

        /// @brief Returns the type of state.
        BaseState::StateFlags get_type() const;

    protected:
        /// @brief Every state needs this.
        SDL::SharedFont m_noto{};

    private:
        /// @brief Stores whether or not the state is still active or can purged.
        bool m_isActive{true};

        /// @brief Whether or not the state is the back of the vector.
        bool m_hasFocus{false};

        /// @brief What type of state this is.
        BaseState::StateFlags m_stateType{};
};

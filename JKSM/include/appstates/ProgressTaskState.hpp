#pragma once
#include "Assets.hpp"
#include "SDL/SDL.hpp"
#include "System/ProgressTask.hpp"
#include "appstates/BaseState.hpp"

#include <memory>

// This is basically like TastState, but it can track progress of a task.
class ProgressTaskState : public BaseState
{
    public:
        template <typename... Args>
        ProgressTaskState(BaseState *creatingState, void (*function)(System::ProgressTask *, Args...), Args... args)
            : BaseState(BaseState::StateFlags::Lock)
            , m_creatingState(creatingState)
            , m_task(std::make_unique<System::ProgressTask>(function, std::forward<Args>(args)...))
            , m_percentageX(160){};

        ~ProgressTaskState() {};

        /// @brief Update override.
        void update() override;

        /// @brief Draw top override.
        void draw_top(SDL_Surface *target);

        /// @brief Draw bottom override.
        void draw_bottom(SDL_Surface *target);

    private:
        /// @brief Pointer to draw state underneath.
        BaseState *m_creatingState{};

        /// @brief Task.
        std::unique_ptr<System::ProgressTask> m_task{};

        /// @brief Current percentage to draw to the screen.
        size_t m_currentPercentage = 0;

        /// @brief Width of the green bar to render.
        double m_LoadBarWidth{};

        /// @brief String representation of the percentage to draw.
        std::string m_percentageString;

        /// @brief Centered X coord to render the percentage to.
        int m_percentageX{};
};

#pragma once
#include "Assets.hpp"
#include "SDL/SDL.hpp"
#include "System/Task.hpp"
#include "appstates/BaseState.hpp"

class TaskState : public BaseState
{
    public:
        template <typename... Args>
        TaskState(BaseState *creatingState, void (*function)(System::Task *, Args...), Args... args)
            : BaseState(BaseState::StateFlags::Lock)
            , m_creatingState(creatingState)
            , m_task(std::make_unique<System::Task>(function, std::forward<Args>(args)...))
        {
        }

        ~TaskState() {};

        /// @brief Update override.
        void update() override;

        /// @brief Draw top override.
        void draw_top(SDL_Surface *target) override;

        /// @brief Draw bottom override.
        void draw_bottom(SDL_Surface *target) override;

    private:
        /// @brief Raw pointer to state creating the task.
        BaseState *m_creatingState{};

        /// @brief Underlying task.
        std::unique_ptr<System::Task> m_task{};
};

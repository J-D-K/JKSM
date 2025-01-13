#pragma once
#include "AppStates/AppState.hpp"
#include "Assets.hpp"
#include "SDL/SDL.hpp"
#include "System/Task.hpp"

class TaskState : public AppState
{
    public:
        template <typename... Args>
        TaskState(AppState *CreatingState, void (*Function)(System::Task *, Args...), Args... Arguments)
            : AppState(AppState::StateFlags::Lock), m_CreatingState(CreatingState),
              m_Task(std::make_unique<System::Task>(Function, std::forward<Args>(Arguments)...))
        {
        }
        ~TaskState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // Raw pointer to state creating the task.
        AppState *m_CreatingState = nullptr;
        // Underlying task.
        std::unique_ptr<System::Task> m_Task;
};

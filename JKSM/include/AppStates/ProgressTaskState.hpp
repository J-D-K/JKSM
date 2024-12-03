#pragma once
#include "AppStates/AppState.hpp"
#include "Assets.hpp"
#include "SDL/SDL.hpp"
#include "System/ProgressTask.hpp"
#include <memory>

// This is basically like TastState, but it can track progress of a task.
class ProgressTaskState : public AppState
{
    public:
        template <typename... Args>
        ProgressTaskState(AppState *CreatingState, void (*Function)(System::ProgressTask *, Args...), Args... Arguments)
            : AppState(AppState::StateFlags::Lock), m_CreatingState(CreatingState),
              m_Task(std::make_unique<System::ProgressTask>(Function, std::forward<Args>(Arguments)...)), m_PercentageX(160)
        {
        }
        ~ProgressTaskState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // This is the raw pointer to the state that created the task so we can call its DrawX functions.
        AppState *m_CreatingState = nullptr;
        // This is the underlying task.
        std::unique_ptr<System::ProgressTask> m_Task;
        // This is the percentage that is grabbed during update.
        size_t m_CurrentPercentage = 0;
        // This is the width of the green bar to show progress.
        double m_LoadBarWidth = 0.0f;
        // This is the string that shows the current percentage done.
        std::string m_PercentageString;
        // This is the X coordinate to blit the percentage string at so it's centered
        int m_PercentageX = 0;
};

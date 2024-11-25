#pragma once
#include "AppStates/AppState.hpp"
#include "Assets.hpp"
#include "SDL/SDL.hpp"
#include "System/Task.hpp"

class TaskState : public AppState
{
    public:
        template <typename... Args>
        TaskState(AppState *CreatingState, void (*Function)(System::Task *, Args...), Args &&...Arguments)
            : m_Task(std::make_unique<System::Task>(Function, std::forward<Args>(Arguments)...)), m_CreatingState(CreatingState)
        {
            // This should almost always be loaded already.
            m_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);
        }
        ~TaskState() {};

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

    private:
        // Underlying task.
        std::unique_ptr<System::Task> m_Task;
        // Raw pointer to state creating the task.
        AppState *m_CreatingState = nullptr;
        // Noto
        SDL::SharedFont m_Noto = nullptr;
};

#include "AppStates/TaskState.hpp"

#include "UI/Draw.hpp"
#include "logging/logger.hpp"

void TaskState::Update(void)
{
    if (m_Task->IsFinished()) { AppState::Deactivate(); }
}

void TaskState::DrawTop(SDL_Surface *Target)
{
    if (m_CreatingState) { m_CreatingState->DrawTop(Target); }
}

void TaskState::DrawBottom(SDL_Surface *Target)
{
    if (m_CreatingState) { m_CreatingState->DrawBottom(Target); }
    // Render dialog box
    UI::DrawDialogBox(Target, 8, 18, 304, 204);
    // Render status here, wrapped. Didn't work too well centered on the top screen.
    std::string ThreadStatus = m_Task->GetStatus();
    m_Noto->BlitTextAt(Target, 30, 30, 12, 268, ThreadStatus.c_str());
}

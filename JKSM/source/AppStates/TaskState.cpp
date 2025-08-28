#include "appstates/TaskState.hpp"

#include "UI/Draw.hpp"
#include "logging/logger.hpp"

void TaskState::update()
{
    if (m_task->IsFinished()) { BaseState::deactivate(); }
}

void TaskState::draw_top(SDL_Surface *target)
{
    if (m_creatingState) { m_creatingState->draw_top(target); }
}

void TaskState::draw_bottom(SDL_Surface *target)
{
    if (m_creatingState) { m_creatingState->draw_bottom(target); }
    // Render dialog box
    UI::DrawDialogBox(target, 8, 18, 304, 204);
    // Render status here, wrapped. Didn't work too well centered on the top screen.
    std::string ThreadStatus = m_task->GetStatus();
    m_noto->BlitTextAt(target, 30, 30, 12, 268, ThreadStatus.c_str());
}

#include "appstates/ProgressTaskState.hpp"

#include "StringUtil.hpp"
#include "UI/Draw.hpp"

#include <cmath>

void ProgressTaskState::update()
{
    if (m_task->IsFinished()) { BaseState::deactivate(); }
    m_currentPercentage = std::ceil(m_task->GetProgress() * 100.0f);
    m_LoadBarWidth      = std::ceil(256.0f * m_task->GetProgress());
    m_percentageString  = StringUtil::GetFormattedString("%u", m_currentPercentage);
    m_percentageX       = 160 - (m_noto->GetTextWidth(12, m_percentageString.c_str()) / 2);
}

void ProgressTaskState::draw_top(SDL_Surface *target)
{
    if (m_creatingState) { m_creatingState->draw_top(target); }
}

void ProgressTaskState::draw_bottom(SDL_Surface *target)
{
    // Bar at top. Just blank.
    SDL::DrawRect(target, 0, 0, 320, 16, SDL::Colors::BarColor);
    // Dialog box and status
    UI::DrawDialogBox(target, 8, 18, 304, 204);
    m_noto->BlitTextAt(target, 30, 30, 12, 268, "%s", m_task->GetStatus().c_str());
    // Bar showing progress.
    SDL::DrawRect(target, 32, 188, 256, 16, SDL::Colors::Black);
    SDL::DrawRect(target, 32, 188, m_LoadBarWidth, 16, SDL::Colors::Green);
    // Text displaying percentage.
    m_noto->BlitTextAt(target, m_percentageX, 189, 12, m_noto->NO_TEXT_WRAP, "%s%%", m_percentageString.c_str());
}

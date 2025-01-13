#include "AppStates/ProgressTaskState.hpp"
#include "StringUtil.hpp"
#include "UI/Draw.hpp"
#include <cmath>

void ProgressTaskState::Update(void)
{
    if (m_Task->IsFinished())
    {
        AppState::Deactivate();
    }
    m_CurrentPercentage = std::ceil(m_Task->GetProgress() * 100.0f);
    m_LoadBarWidth = std::ceil(256.0f * m_Task->GetProgress());
    m_PercentageString = StringUtil::GetFormattedString("%u", m_CurrentPercentage);
    m_PercentageX = 160 - (m_Noto->GetTextWidth(12, m_PercentageString.c_str()) / 2);
}

void ProgressTaskState::DrawTop(SDL_Surface *Target)
{
    if (m_CreatingState)
    {
        m_CreatingState->DrawTop(Target);
    }
}

void ProgressTaskState::DrawBottom(SDL_Surface *Target)
{
    // Bar at top. Just blank.
    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    // Dialog box and status
    UI::DrawDialogBox(Target, 8, 18, 304, 204);
    m_Noto->BlitTextAt(Target, 30, 30, 12, 268, "%s", m_Task->GetStatus().c_str());
    // Bar showing progress.
    SDL::DrawRect(Target, 32, 188, 256, 16, SDL::Colors::Black);
    SDL::DrawRect(Target, 32, 188, m_LoadBarWidth, 16, SDL::Colors::Green);
    // Text displaying percentage.
    m_Noto->BlitTextAt(Target, m_PercentageX, 189, 12, m_Noto->NO_TEXT_WRAP, "%s%%", m_PercentageString.c_str());
}

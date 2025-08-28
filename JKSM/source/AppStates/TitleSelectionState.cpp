#include "appstates/TitleSelectionState.hpp"

#include "Assets.hpp"
#include "Input.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"

#include <array>

TitleSelectionState::TitleSelectionState(Data::SaveDataType saveType)
    : BaseSelectionState(saveType)
    , m_titleView(saveType)
{
    m_textX = 200 - (m_noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::StateName, m_saveType)) / 2);
}

void TitleSelectionState::update()
{
    m_titleView.Update();

    if (Input::ButtonPressed(KEY_A)) { BaseSelectionState::create_backup_state(m_titleView.GetSelectedTitleData()); }
    else if (Input::ButtonPressed(KEY_X)) { BaseSelectionState::create_option_state(m_titleView.GetSelectedTitleData()); }
}

void TitleSelectionState::draw_top(SDL_Surface *target)
{
    m_titleView.Draw(target);
    SDL::DrawRect(target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_noto->BlitTextAt(target,
                       m_textX,
                       225,
                       12,
                       m_noto->NO_TEXT_WRAP,
                       Strings::GetStringByName(Strings::Names::StateName, m_saveType));
}

void TitleSelectionState::draw_bottom(SDL_Surface *target)
{
    BaseSelectionState::draw_title_info(target, m_titleView.GetSelectedTitleData());
}

void TitleSelectionState::refresh() { m_titleView.Refresh(); }

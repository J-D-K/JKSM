#include "appstates/TextTitleSelect.hpp"

#include "Assets.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"
#include "input.hpp"

TextTitleSelect::TextTitleSelect(Data::SaveDataType saveType)
    : BaseSelectionState(saveType)
    , m_titleMenu(40, 20, 320, 12)
{
    const char *stateName = Strings::GetStringByName(Strings::Names::StateName, m_saveType);

    m_textX = 200 - (m_noto->GetTextWidth(12, stateName) / 2);

    TextTitleSelect::refresh();
}

void TextTitleSelect::update()
{
    m_titleMenu.Update();

    if (input::button_pressed(KEY_A)) { BaseSelectionState::create_backup_state(m_titleData.at(m_titleMenu.GetSelected())); }
}

void TextTitleSelect::draw_top(SDL_Surface *target)
{
    m_titleMenu.Draw(target);
    SDL::DrawRect(target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_noto->BlitTextAt(target,
                       m_textX,
                       225,
                       12,
                       m_noto->NO_TEXT_WRAP,
                       Strings::GetStringByName(Strings::Names::StateName, m_saveType));
}

void TextTitleSelect::draw_bottom(SDL_Surface *target)
{
    BaseSelectionState::draw_title_info(target, m_titleData.at(m_titleMenu.GetSelected()));
}

void TextTitleSelect::refresh()
{
    m_titleMenu.Reset();

    Data::GetTitlesWithType(m_saveType, m_titleData);

    for (auto &currentData : m_titleData)
    {
        char UTF8Title[0x80] = {0};
        StringUtil::ToUTF8(currentData->GetTitle(), UTF8Title, 0x80);
        m_titleMenu.AddOption(UTF8Title);
    }
}

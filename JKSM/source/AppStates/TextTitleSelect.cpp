#include "AppStates/TextTitleSelect.hpp"
#include "Assets.hpp"
#include "Input.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"

TextTitleSelect::TextTitleSelect(Data::SaveDataType SaveType) : BaseSelectionState(SaveType), m_TitleMenu(40, 20, 320, 12)
{
    m_TextX = 200 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::StateName, m_SaveType)) / 2);

    TextTitleSelect::Refresh();
}

void TextTitleSelect::Update(void)
{
    m_TitleMenu.Update();

    if (Input::ButtonPressed(KEY_A))
    {
        BaseSelectionState::CreateBackupStateWithData(m_TitleData.at(m_TitleMenu.GetSelected()));
    }
}

void TextTitleSelect::DrawTop(SDL_Surface *Target)
{
    m_TitleMenu.Draw(Target);
    SDL::DrawRect(Target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_TextX, 225, 12, m_Noto->NO_TEXT_WRAP, UI::Strings::GetStringByName(UI::Strings::Names::StateName, m_SaveType));
}

void TextTitleSelect::DrawBottom(SDL_Surface *Target)
{
    BaseSelectionState::DrawTitleInformation(Target, m_TitleData.at(m_TitleMenu.GetSelected()));
}

void TextTitleSelect::Refresh(void)
{
    m_TitleMenu.Reset();

    Data::GetTitlesWithType(m_SaveType, m_TitleData);

    for (auto &CurrentData : m_TitleData)
    {
        char UTF8Title[0x80] = {0};
        StringUtil::ToUTF8(CurrentData->GetTitle(), UTF8Title, 0x80);
        m_TitleMenu.AddOption(UTF8Title);
    }
}

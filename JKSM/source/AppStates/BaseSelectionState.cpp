#include "AppStates/BaseSelectionState.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"

void BaseSelectionState::DrawTitleInformation(SDL_Surface *Target, const Data::TitleData *Data)
{
    char UTF8Title[0x80] = {0};
    char UTF8Publisher[0x80] = {0};
    StringUtil::ToUTF8(Data->GetTitle(), UTF8Title, 0x80);
    StringUtil::ToUTF8(Data->GetPublisher(), UTF8Publisher, 0x80);

    // This is to center the title above the information.
    int TitleX = 160 - (m_Noto->GetTextWidth(12, UTF8Title) / 2);

    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, TitleX, 1, 12, m_Noto->NO_TEXT_WRAP, UTF8Title);
    m_Noto->BlitTextAt(Target,
                       4,
                       18,
                       12,
                       320,
                       UI::Strings::GetStringByName(UI::Strings::Names::StateInformation, 0),
                       Data->GetTitleID(),
                       UTF8Publisher,
                       UI::Strings::GetStringByName(UI::Strings::Names::MediaType, Data->GetMediaType()),
                       Data->GetProductCode());
}

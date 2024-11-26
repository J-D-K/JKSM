#include "AppStates/TitleSelectionState.hpp"
#include "Assets.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"
#include <array>

TitleSelectionState::TitleSelectionState(Data::SaveDataType SaveType)
    : m_TitleView(std::make_unique<UI::TitleView>(SaveType)), m_SaveType(SaveType)
{
    // This should already be loaded and ready to go from boot. I shouldn't assume that, but I will.
    m_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);

    m_TextX = 200 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::StateName, m_SaveType)) / 2);
}

void TitleSelectionState::Update(void)
{
    m_TitleView->Update();
}

void TitleSelectionState::DrawTop(SDL_Surface *Target)
{
    m_TitleView->Draw(Target);
    SDL::DrawRect(Target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_TextX, 225, 12, m_Noto->NO_TEXT_WRAP, UI::Strings::GetStringByName(UI::Strings::Names::StateName, m_SaveType));
}

void TitleSelectionState::DrawBottom(SDL_Surface *Target)
{
    Data::TitleData *SelectedTitleData = m_TitleView->GetSelectedTitleData();

    char UTF8Title[0x80] = {0};
    StringUtil::ToUTF8(SelectedTitleData->GetTitle(), UTF8Title, 0x80);
    int TitleX = 160 - (m_Noto->GetTextWidth(12, UTF8Title) / 2);

    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, TitleX, 1, 12, m_Noto->NO_TEXT_WRAP, UTF8Title);
    m_Noto->BlitTextAt(Target,
                       4,
                       18,
                       12,
                       320,
                       UI::Strings::GetStringByName(UI::Strings::Names::StateInformation, 0),
                       SelectedTitleData->GetTitleID(),
                       UI::Strings::GetStringByName(UI::Strings::Names::MediaType, SelectedTitleData->GetMediaType()),
                       SelectedTitleData->GetProductCode());
}

void TitleSelectionState::Refresh(void)
{
    m_TitleView->Refresh();
}

#include "AppStates/TitleSelectionState.hpp"
#include "Assets.hpp"
#include "StringUtil.hpp"
#include <array>

namespace
{
    constexpr std::array<std::string_view, Data::SaveTypeTotal> s_SaveTypeStrings = {"User Saves",
                                                                                     "ExtData Saves",
                                                                                     "System Saves",
                                                                                     "BOSS ExtData Saves"};

    constexpr std::array<std::string_view, 3> s_MediaTypeStrings = {"NAND", "SD Card", "Game Card"};
} // namespace

TitleSelectionState::TitleSelectionState(Data::SaveDataType SaveType)
    : m_TitleView(std::make_unique<UI::TitleView>(SaveType)), m_SaveType(SaveType)
{
    // This should already be loaded and ready to go from boot. I shouldn't assume that, but I will.
    m_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);

    m_TextX = 200 - (m_Noto->GetTextWidth(12, s_SaveTypeStrings[SaveType].data()) / 2);
}

void TitleSelectionState::Update(void)
{
    m_TitleView->Update();
}

void TitleSelectionState::DrawTop(SDL_Surface *Target)
{
    m_TitleView->Draw(Target);
    SDL::DrawRect(Target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_TextX, 225, 12, SDL::Font::NO_TEXT_WRAP, s_SaveTypeStrings[m_SaveType].data());
}

void TitleSelectionState::DrawBottom(SDL_Surface *Target)
{
    Data::TitleData *SelectedTitleData = m_TitleView->GetSelectedTitleData();

    std::string GameTitle = StringUtil::ToUTF8(SelectedTitleData->GetTitle());
    int TitleX = 160 - (m_Noto->GetTextWidth(12, GameTitle.c_str()) / 2);

    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, TitleX, 1, 12, SDL::Font::NO_TEXT_WRAP, GameTitle.c_str());
    m_Noto->BlitTextAt(Target,
                       4,
                       18,
                       12,
                       320,
                       "Title ID: %016llX\nMedia Type: %s",
                       SelectedTitleData->GetTitleID(),
                       s_MediaTypeStrings[SelectedTitleData->GetMediaType()].data());
}

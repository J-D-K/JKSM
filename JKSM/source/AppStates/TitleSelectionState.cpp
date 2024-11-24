#include "AppStates/TitleSelectionState.hpp"
#include "StringUtil.hpp"
#include <array>

namespace
{
    constexpr std::array<std::string_view, Data::SaveTypeTotal> s_SaveTypeStrings = {"User Saves",
                                                                                     "ExtData Saves",
                                                                                     "System Saves",
                                                                                     "BOSS ExtData Saves"};
}

TitleSelectionState::TitleSelectionState(Data::SaveDataType SaveType)
    : m_TitleView(std::make_unique<UI::TitleView>(SaveType)), m_SaveType(SaveType)
{
    // This should already be loaded and ready to go from boot.
    m_Noto = SDL::FontManager::CreateLoadResource("NotoSans", "romfs:/NotoSansJP-Medium.ttf", static_cast<SDL::Color>(0xFFFFFFFF));

    m_TextX = 200 - (m_Noto->GetTextWidth(12, s_SaveTypeStrings[SaveType].data()) / 2);
}

void TitleSelectionState::Update(void)
{
    m_TitleView->Update();
}

void TitleSelectionState::DrawTop(SDL_Surface *Target)
{
    m_TitleView->Draw(Target);
    SDL::DrawRect(Target, 0, 224, 400, 16, {0x3D3D3DFF});
    m_Noto->BlitTextAt(Target, m_TextX, 225, 12, SDL::Font::NO_TEXT_WRAP, s_SaveTypeStrings[m_SaveType].data());
}

void TitleSelectionState::DrawBottom(SDL_Surface *Target)
{
    Data::TitleData *SelectedTitleData = m_TitleView->GetSelectedTitleData();

    m_Noto->BlitTextAt(Target,
                       0,
                       0,
                       12,
                       320,
                       "Title: %s\nTitle ID: %016llX",
                       StringUtil::ToUTF8(SelectedTitleData->GetTitle()).c_str(),
                       SelectedTitleData->GetTitleID());
}

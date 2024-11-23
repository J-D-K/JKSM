#include "AppStates/TitleSelectionState.hpp"

TitleSelectionState::TitleSelectionState(Data::SaveDataType SaveType) : m_TitleView(std::make_unique<UI::TitleView>(SaveType))
{
    // This should already be loaded and ready to go from boot.
    m_Noto = SDL::FontManager::CreateLoadResource("NotoSans", "romfs:/NotoSansJP-Medium.ttf", static_cast<SDL::Color>(0xFFFFFFFF));
}

void TitleSelectionState::Update(void)
{
    m_TitleView->Update();
}

void TitleSelectionState::DrawTop(SDL_Surface *Target)
{
    m_TitleView->Draw(Target);
}

void TitleSelectionState::DrawBottom(SDL_Surface *Target)
{
    m_Noto->BlitTextAt(Target, 0, 0, 12, SDL::Font::NO_TEXT_WRAP, "Test Text");
}

#include "AppStates/TextTitleSelect.hpp"
#include "Assets.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"

TextTitleSelect::TextTitleSelect(Data::SaveDataType SaveType) : m_TitleMenu(std::make_unique<UI::Menu>(40, 18, 320, 13)), m_SaveType(SaveType)
{
    // Grab pointers to data by type
    Data::GetTitlesWithType(m_SaveType, m_TitleData);
    // Make sure we have the font.
    m_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);

    m_TextX = 200 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::StateName, m_SaveType)) / 2);

    // Copy titles of games to menu
    for (auto &CurrentTitle : m_TitleData)
    {
        char UTF8Title[0x80] = {0};
        StringUtil::ToUTF8(CurrentTitle->GetTitle(), UTF8Title, 0x80);
        m_TitleMenu->AddOption(UTF8Title);
    }
}

void TextTitleSelect::Update(void)
{
    m_TitleMenu->Update();
}

void TextTitleSelect::DrawTop(SDL_Surface *Target)
{
    m_TitleMenu->Draw(Target);
    SDL::DrawRect(Target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_TextX, 225, 12, m_Noto->NO_TEXT_WRAP, UI::Strings::GetStringByName(UI::Strings::Names::StateName, m_SaveType));
}

void TextTitleSelect::DrawBottom(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
}

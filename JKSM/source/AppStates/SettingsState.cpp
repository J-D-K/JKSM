#include "AppStates/SettingsState.hpp"
#include "Assets.hpp"
#include "Logger.hpp"
#include "UI/Strings.hpp"
#include <array>
#include <string_view>

SettingsState::SettingsState(void)
    : m_SettingsMenu(std::make_unique<UI::Menu>(40, 18, 320, 12)),
      m_Noto(SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White))
{
    // Get centered coordinates for text.
    m_SettingsX = 200 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::StateName, 5)) / 2);
    m_DescriptionX = 160 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::SettingsDescription, 0)) / 2);

    uint8_t CurrentIndex = 0;
    const char *SettingsOption = NULL;
    while ((SettingsOption = UI::Strings::GetStringByName(UI::Strings::Names::SettingsMenu, CurrentIndex++)) != NULL)
    {
        m_SettingsMenu->AddOption(SettingsOption);
    }
}

void SettingsState::Update(void)
{
    m_SettingsMenu->Update();
}

void SettingsState::DrawTop(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_SettingsX, 225, 12, m_Noto->NO_TEXT_WRAP, UI::Strings::GetStringByName(UI::Strings::Names::StateName, 5));
    m_SettingsMenu->Draw(Target);
}

void SettingsState::DrawBottom(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target,
                       m_DescriptionX,
                       1,
                       12,
                       m_Noto->NO_TEXT_WRAP,
                       UI::Strings::GetStringByName(UI::Strings::Names::SettingsDescription, 0));

    m_Noto->BlitTextAt(Target,
                       4,
                       18,
                       12,
                       312,
                       UI::Strings::GetStringByName(UI::Strings::Names::SettingsDecriptions, m_SettingsMenu->GetSelected()));
}

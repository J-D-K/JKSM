#include "AppStates/SettingsState.hpp"
#include "Assets.hpp"
#include "Logger.hpp"
#include <array>
#include <string_view>

namespace
{
    // Text displayed on bottom of top screen.
    constexpr std::string_view SETTINGS_TEXT = "Settings";
    // Text displayed on top of bottom screen.
    constexpr std::string_view DESCRIPTION_TEXT = "Description";
    // Total number of settings available
    constexpr size_t TOTAL_SETTINGS = 3;
    // These are the actual strings for the menu options.
    constexpr std::array<std::string_view, TOTAL_SETTINGS> s_SettingsStrings = {"Refresh Title List", "Text Menus: %s", "Use ZIP: %s"};
    // These are the descriptions of what each one does.
    constexpr std::array<std::string_view, TOTAL_SETTINGS> s_SettingDescriptions = {"Rescans system for new titles and caches them.",
                                                                                    "Uses text menus instead of icon grids.",
                                                                                    "Uses ZIP for backups instead of unpacked folders."};
} // namespace

SettingsState::SettingsState(void)
    : m_SettingsMenu(std::make_unique<UI::Menu>(40, 18, 320, TOTAL_SETTINGS, s_SettingsStrings.data(), TOTAL_SETTINGS)),
      m_Noto(SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White))
{
    m_SettingsX = 200 - (m_Noto->GetTextWidth(12, SETTINGS_TEXT.data()) / 2);
    m_DescriptionX = 160 - (m_Noto->GetTextWidth(12, DESCRIPTION_TEXT.data()) / 2);
}

void SettingsState::Update(void)
{
    m_SettingsMenu->Update();
}

void SettingsState::DrawTop(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_SettingsX, 225, 12, SDL::Font::NO_TEXT_WRAP, SETTINGS_TEXT.data());
}

void SettingsState::DrawBottom(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_DescriptionX, 1, 12, m_Noto->NO_TEXT_WRAP, DESCRIPTION_TEXT.data());
    m_Noto->BlitTextAt(Target, 4, 18, 12, 312, s_SettingDescriptions.at(m_SettingsMenu->GetSelected()).data());
}

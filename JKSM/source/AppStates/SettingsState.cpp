#include "AppStates/SettingsState.hpp"
#include "AppStates/ProgressTaskState.hpp"
#include "Assets.hpp"
#include "Config.hpp"
#include "Data/Data.hpp"
#include "FsLib.hpp"
#include "Input.hpp"
#include "JKSM.hpp"
#include "Logger.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"
#include <3ds.h>
#include <array>
#include <string_view>

// This enum is to make the switch case easier to read.
enum
{
    REFRESH_TITLES,
    TEXT_MODE,
    EXPORT_TO_ZIP,
    FORCE_ENGLISH
};

// This doesn't really convert bools, but tha
static const char *GetValueText(uint8_t Value)
{
    switch (Value)
    {
        case 0:
        {
            return "Off";
        }
        break;

        case 1:
        {
            return "On";
        }
        break;
    }
    return "nullptr";
}

SettingsState::SettingsState(void) : m_SettingsMenu(std::make_unique<UI::Menu>(40, 20, 320, 12))
{
    // Get centered coordinates for text.
    m_SettingsX = 200 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::StateName, 5)) / 2);
    m_DescriptionX = 160 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::SettingsDescription, 0)) / 2);

    uint8_t CurrentIndex = 0;
    const char *SettingsOption = NULL;
    while ((SettingsOption = UI::Strings::GetStringByName(UI::Strings::Names::SettingsMenu, CurrentIndex++)) != nullptr)
    {
        // This is not the way I should do this, but I need to for now.
        m_SettingsMenu->AddOption(StringUtil::GetFormattedString(SettingsOption, GetValueText(0)));
    }
}

void SettingsState::Update(void)
{
    m_SettingsMenu->Update();
    SettingsState::UpdateMenuStrings();
    if (Input::ButtonPressed(KEY_A))
    {
        SettingsState::UpdateConfig();
    }
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

void SettingsState::UpdateMenuStrings(void)
{
    // Wew these are long...
    m_SettingsMenu->EditOption(1,
                               StringUtil::GetFormattedString(UI::Strings::GetStringByName(UI::Strings::Names::SettingsMenu, 1),
                                                              GetValueText(Config::GetByKey(Config::Keys::TextMode))));
    m_SettingsMenu->EditOption(2,
                               StringUtil::GetFormattedString(UI::Strings::GetStringByName(UI::Strings::Names::SettingsMenu, 2),
                                                              GetValueText(Config::GetByKey(Config::Keys::ExportToZip))));
    m_SettingsMenu->EditOption(3,
                               StringUtil::GetFormattedString(UI::Strings::GetStringByName(UI::Strings::Names::SettingsMenu, 3),
                                                              GetValueText(Config::GetByKey(Config::Keys::ForceEnglish))));
}

void SettingsState::UpdateConfig(void)
{
    bool SaveConfg = true;
    switch (m_SettingsMenu->GetSelected())
    {
        case REFRESH_TITLES:
        {
            if (FsLib::DeleteFile(u"sdmc:/JKSM/cache.bin"))
            {
                std::shared_ptr<AppState> DataRefresh = std::make_shared<ProgressTaskState>(this, Data::Initialize);
                JKSM::PushState(DataRefresh);
            }
            SaveConfg = false;
        }
        break;

        case TEXT_MODE:
        {
            Config::SetByKey(Config::Keys::TextMode, Config::GetByKey(Config::Keys::TextMode) ? 0 : 1);
            JKSM::InitializeTitleViewStates();
        }
        break;

        case EXPORT_TO_ZIP:
        {
            Config::SetByKey(Config::Keys::ExportToZip, Config::GetByKey(Config::Keys::ExportToZip) ? 0 : 1);
        }
        break;

        case FORCE_ENGLISH:
        {
            Config::SetByKey(Config::Keys::ForceEnglish, Config::GetByKey(Config::Keys::ForceEnglish) ? 0 : 1);
        }
        break;
    }

    if (SaveConfg)
    {
        Config::Save();
    }
}

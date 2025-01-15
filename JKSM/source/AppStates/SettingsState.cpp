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
#include "Strings.hpp"
#include <3ds.h>
#include <array>
#include <string_view>

// This enum is to make the switch case easier to read.
enum
{
    REFRESH_TITLES,
    TEXT_MODE,
    EXPORT_TO_ZIP,
    FORCE_ENGLISH,
    PRESERVE_SECURE_VALUE,
    HOLD_FOR_OVERWRITE,
    HOLD_FOR_RESTORE,
    HOLD_FOR_DELETION
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

SettingsState::SettingsState(void) : m_SettingsMenu(40, 20, 320, 12)
{
    // Get centered coordinates for text.
    m_SettingsX = 200 - (m_Noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::StateName, 5)) / 2);
    m_DescriptionX = 160 - (m_Noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::SettingsDescription, 0)) / 2);

    uint8_t CurrentIndex = 0;
    const char *SettingsOption = NULL;
    while ((SettingsOption = Strings::GetStringByName(Strings::Names::SettingsMenu, CurrentIndex++)) != nullptr)
    {
        // This is not the way I should do this, but I need to for now.
        m_SettingsMenu.AddOption(StringUtil::GetFormattedString(SettingsOption, GetValueText(0)));
    }
}

void SettingsState::Update(void)
{
    m_SettingsMenu.Update();
    SettingsState::UpdateMenuStrings();
    if (Input::ButtonPressed(KEY_A))
    {
        SettingsState::UpdateConfig();
    }
}

void SettingsState::DrawTop(SDL_Surface *Target)
{
    m_SettingsMenu.Draw(Target);
    SDL::DrawRect(Target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_SettingsX, 225, 12, m_Noto->NO_TEXT_WRAP, Strings::GetStringByName(Strings::Names::StateName, 5));
}

void SettingsState::DrawBottom(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, m_DescriptionX, 1, 12, m_Noto->NO_TEXT_WRAP, Strings::GetStringByName(Strings::Names::SettingsDescription, 0));

    m_Noto->BlitTextAt(Target, 4, 18, 12, 312, Strings::GetStringByName(Strings::Names::SettingsDescriptions, m_SettingsMenu.GetSelected()));
}

void SettingsState::UpdateMenuStrings(void)
{
    // Wew these are long...
    m_SettingsMenu.EditOption(1,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 1),
                                                             GetValueText(Config::GetByKey(Config::Keys::TextMode))));
    m_SettingsMenu.EditOption(2,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 2),
                                                             GetValueText(Config::GetByKey(Config::Keys::ExportToZip))));
    m_SettingsMenu.EditOption(3,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 3),
                                                             GetValueText(Config::GetByKey(Config::Keys::ForceEnglish))));
    m_SettingsMenu.EditOption(4,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 4),
                                                             GetValueText(Config::GetByKey(Config::Keys::PreserveSecureValues))));
    m_SettingsMenu.EditOption(5,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 5),
                                                             GetValueText(Config::GetByKey(Config::Keys::HoldToOverwrite))));
    m_SettingsMenu.EditOption(6,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 6),
                                                             GetValueText(Config::GetByKey(Config::Keys::HoldToRestore))));
    m_SettingsMenu.EditOption(7,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 7),
                                                             GetValueText(Config::GetByKey(Config::Keys::HoldToDelete))));
}

void SettingsState::UpdateConfig(void)
{
    bool SaveConfig = true;
    switch (m_SettingsMenu.GetSelected())
    {
        case REFRESH_TITLES:
        {
            if (FsLib::DeleteFile(u"sdmc:/JKSM/cache.bin"))
            {
                std::shared_ptr<AppState> DataRefresh = std::make_shared<ProgressTaskState>(this, Data::Initialize);
                JKSM::PushState(DataRefresh);
            }
            SaveConfig = false;
        }
        break;

        case TEXT_MODE:
        {
            Config::SetByKey(Config::Keys::TextMode, Config::GetByKey(Config::Keys::TextMode) ? 0 : 1);
            JKSM::InitializeViews();
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

        case PRESERVE_SECURE_VALUE:
        {
            Config::SetByKey(Config::Keys::PreserveSecureValues, Config::GetByKey(Config::Keys::PreserveSecureValues) ? 0 : 1);
        }
        break;

        case HOLD_FOR_OVERWRITE:
        {
            Config::SetByKey(Config::Keys::HoldToOverwrite, Config::GetByKey(Config::Keys::HoldToOverwrite) ? 0 : 1);
        }
        break;

        case HOLD_FOR_RESTORE:
        {
            Config::SetByKey(Config::Keys::HoldToRestore, Config::GetByKey(Config::Keys::HoldToRestore) ? 0 : 1);
        }
        break;

        case HOLD_FOR_DELETION:
        {
            Config::SetByKey(Config::Keys::HoldToDelete, Config::GetByKey(Config::Keys::HoldToDelete) ? 0 : 1);
        }
        break;
    }

    if (SaveConfig)
    {
        Config::Save();
    }
}

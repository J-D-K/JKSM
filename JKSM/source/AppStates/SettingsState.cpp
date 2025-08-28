#include "appstates/SettingsState.hpp"

#include "Assets.hpp"
#include "Config.hpp"
#include "Data/Data.hpp"
#include "JKSM.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"
#include "appstates/ProgressTaskState.hpp"
#include "fslib.hpp"
#include "input.hpp"
#include "logging/logger.hpp"

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

SettingsState::SettingsState()
    : m_settingsMenu(40, 20, 320, 12)
{
    // Get centered coordinates for text.
    m_settingsX    = 200 - (m_noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::StateName, 5)) / 2);
    m_descriptionX = 160 - (m_noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::SettingsDescription, 0)) / 2);

    uint8_t CurrentIndex       = 0;
    const char *SettingsOption = NULL;
    while ((SettingsOption = Strings::GetStringByName(Strings::Names::SettingsMenu, CurrentIndex++)) != nullptr)
    {
        // This is not the way I should do this, but I need to for now.
        m_settingsMenu.AddOption(StringUtil::GetFormattedString(SettingsOption, GetValueText(0)));
    }
}

void SettingsState::update()
{
    m_settingsMenu.Update();
    SettingsState::update_menu_strings();
    if (input::button_pressed(KEY_A)) { SettingsState::update_config(); }
}

void SettingsState::draw_top(SDL_Surface *target)
{
    m_settingsMenu.Draw(target);
    SDL::DrawRect(target, 0, 224, 400, 16, SDL::Colors::BarColor);
    m_noto->BlitTextAt(target,
                       m_settingsX,
                       225,
                       12,
                       m_noto->NO_TEXT_WRAP,
                       Strings::GetStringByName(Strings::Names::StateName, 5));
}

void SettingsState::draw_bottom(SDL_Surface *target)
{
    SDL::DrawRect(target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_noto->BlitTextAt(target,
                       m_descriptionX,
                       1,
                       12,
                       m_noto->NO_TEXT_WRAP,
                       Strings::GetStringByName(Strings::Names::SettingsDescription, 0));

    m_noto->BlitTextAt(target,
                       4,
                       18,
                       12,
                       312,
                       Strings::GetStringByName(Strings::Names::SettingsDescriptions, m_settingsMenu.GetSelected()));
}

void SettingsState::update_menu_strings()
{
    // Wew these are long...
    m_settingsMenu.EditOption(1,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 1),
                                                             GetValueText(Config::GetByKey(Config::Keys::TextMode))));
    m_settingsMenu.EditOption(2,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 2),
                                                             GetValueText(Config::GetByKey(Config::Keys::ExportToZip))));
    m_settingsMenu.EditOption(3,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 3),
                                                             GetValueText(Config::GetByKey(Config::Keys::ForceEnglish))));
    m_settingsMenu.EditOption(
        4,
        StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 4),
                                       GetValueText(Config::GetByKey(Config::Keys::PreserveSecureValues))));
    m_settingsMenu.EditOption(5,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 5),
                                                             GetValueText(Config::GetByKey(Config::Keys::HoldToOverwrite))));
    m_settingsMenu.EditOption(6,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 6),
                                                             GetValueText(Config::GetByKey(Config::Keys::HoldToRestore))));
    m_settingsMenu.EditOption(7,
                              StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::SettingsMenu, 7),
                                                             GetValueText(Config::GetByKey(Config::Keys::HoldToDelete))));
}

void SettingsState::update_config()
{
    bool SaveConfig = true;
    switch (m_settingsMenu.GetSelected())
    {
        case REFRESH_TITLES:
        {
            if (fslib::delete_file(u"sdmc:/JKSM/cache.bin"))
            {
                std::shared_ptr<BaseState> DataRefresh = std::make_shared<ProgressTaskState>(this, Data::Initialize);
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

    if (SaveConfig) { Config::Save(); }
}

#include "appstates/BaseSelectionState.hpp"

#include "FS/SaveMount.hpp"
#include "JKSM.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"
#include "appstates/BackupMenuState.hpp"
#include "appstates/TitleOptionState.hpp"
#include "fslib.hpp"
#include "logging/logger.hpp"

#include <memory>

BaseSelectionState::BaseSelectionState(Data::SaveDataType saveType)
    : m_saveType(saveType) {};

void BaseSelectionState::draw_title_info(SDL_Surface *target, const Data::TitleData *data)
{
    const char16_t *title     = data->GetTitle();
    const char16_t *publisher = data->GetPublisher();
    char utf8Title[0x80]      = {0};
    char utf8Publisher[0x80]  = {0};
    StringUtil::ToUTF8(title, utf8Title, 0x80);
    StringUtil::ToUTF8(publisher, utf8Publisher, 0x80);

    const int titleX        = 160 - (m_noto->GetTextWidth(12, utf8Title) / 2);
    const char *infoFormat  = Strings::GetStringByName(Strings::Names::StateInformation, 0);
    const char *mediaType   = Strings::GetStringByName(Strings::Names::MediaType, data->GetMediaType());
    const uint64_t titleID  = data->GetTitleID();
    const char *productCode = data->GetProductCode();

    SDL::DrawRect(target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_noto->BlitTextAt(target, titleX, 1, 12, m_noto->NO_TEXT_WRAP, utf8Title);
    m_noto->BlitTextAt(target, 4, 18, 12, 320, infoFormat, titleID, utf8Publisher, mediaType, productCode);
}

void BaseSelectionState::create_backup_state(const Data::TitleData *data)
{
    if (!BaseSelectionState::mount_save_data(data)) { return; }

    auto newState = std::make_shared<TitleOptionState>(this, data, m_saveType);
    JKSM::PushState(newState);
}

void BaseSelectionState::create_option_state(const Data::TitleData *data)
{
    auto newState = std::make_shared<TitleOptionState>(this, data, m_saveType);
    JKSM::PushState(newState);
}

bool BaseSelectionState::mount_save_data(const Data::TitleData *data)
{
    bool mounted = false;
    switch (m_saveType)
    {
        case Data::SaveTypeUser:
        {
            mounted = fslib::open_user_save_data(FS::SAVE_MOUNT, data->GetMediaType(), data->GetTitleID());
        }
        break;

        case Data::SaveTypeExtData:
        {
            mounted = fslib::open_extra_data(FS::SAVE_MOUNT, data->GetExtDataID());
        }
        break;

        case Data::SaveTypeSharedExtData:
        {
            mounted = fslib::open_shared_extra_data(FS::SAVE_MOUNT, data->GetLowerID());
        }
        break;

        case Data::SaveTypeBossExtData:
        {
            mounted = fslib::open_boss_extra_data(FS::SAVE_MOUNT, data->GetExtDataID());
        }
        break;

        case Data::SaveTypeSystem:
        {
            mounted = fslib::open_system_save_data(FS::SAVE_MOUNT, data->GetUniqueID());
        }
        break;

        default:
        {
        }
        break;
    }

    if (!mounted) { logger::log("Error mounting save for %016llX: %s", data->GetTitleID(), fslib::error::get_string()); }

    return mounted;
}

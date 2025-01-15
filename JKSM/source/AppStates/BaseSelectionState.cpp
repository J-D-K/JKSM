#include "AppStates/BaseSelectionState.hpp"
#include "AppStates/BackupMenuState.hpp"
#include "AppStates/TitleOptionState.hpp"
#include "FS/SaveMount.hpp"
#include "FsLib.hpp"
#include "JKSM.hpp"
#include "Logger.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"
#include <memory>

void BaseSelectionState::DrawTitleInformation(SDL_Surface *Target, const Data::TitleData *Data)
{
    char UTF8Title[0x80] = {0};
    char UTF8Publisher[0x80] = {0};
    StringUtil::ToUTF8(Data->GetTitle(), UTF8Title, 0x80);
    StringUtil::ToUTF8(Data->GetPublisher(), UTF8Publisher, 0x80);

    // This is to center the title above the information.
    int TitleX = 160 - (m_Noto->GetTextWidth(12, UTF8Title) / 2);

    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target, TitleX, 1, 12, m_Noto->NO_TEXT_WRAP, UTF8Title);
    m_Noto->BlitTextAt(Target,
                       4,
                       18,
                       12,
                       320,
                       UI::Strings::GetStringByName(UI::Strings::Names::StateInformation, 0),
                       Data->GetTitleID(),
                       UTF8Publisher,
                       UI::Strings::GetStringByName(UI::Strings::Names::MediaType, Data->GetMediaType()),
                       Data->GetProductCode());
}

void BaseSelectionState::CreateBackupStateWithData(const Data::TitleData *Data)
{
    if (!MountSaveData(Data))
    {
        return;
    }

    JKSM::PushState(std::make_shared<BackupMenuState>(this, Data, m_SaveType));
}

void BaseSelectionState::CreateOptionStateWithData(const Data::TitleData *Data)
{
    JKSM::PushState(std::make_shared<TitleOptionState>(this, Data, m_SaveType));
}

bool BaseSelectionState::MountSaveData(const Data::TitleData *Data)
{
    bool Mounted = false;
    switch (m_SaveType)
    {
        case Data::SaveTypeUser:
        {
            Mounted = FsLib::OpenUserSaveData(FS::SAVE_MOUNT, Data->GetMediaType(), Data->GetLowerID(), Data->GetUpperID());
        }
        break;

        case Data::SaveTypeExtData:
        {
            Mounted = FsLib::OpenExtData(FS::SAVE_MOUNT, Data->GetExtDataID());
        }
        break;

        case Data::SaveTypeSharedExtData:
        {
            Mounted = FsLib::OpenSharedExtData(FS::SAVE_MOUNT, Data->GetLowerID());
        }
        break;

        case Data::SaveTypeBossExtData:
        {
            Mounted = FsLib::OpenBossExtData(FS::SAVE_MOUNT, Data->GetExtDataID());
        }
        break;

        case Data::SaveTypeSystem:
        {
            Mounted = FsLib::OpenSystemSaveData(FS::SAVE_MOUNT, Data->GetUniqueID());
        }
        break;

        default:
        {
        }
        break;
    }

    if (!Mounted)
    {
        Logger::Log("Error mounting save for %016llX: %s", Data->GetTitleID(), FsLib::GetErrorString());
    }

    return Mounted;
}

#include "AppStates/TitleOptionState.hpp"
#include "AppStates/ConfirmState.hpp"
#include "AppStates/MessageState.hpp"
#include "FS/FS.hpp"
#include "FS/SaveMount.hpp"
#include "FsLib.hpp"
#include "Input.hpp"
#include "JKSM.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"
#include "System/Task.hpp"
#include "UI/Draw.hpp"

namespace
{
    // Enums for menu
    enum
    {
        DELETE_SECURE_VALUE,
        ERASE_SAVE_DATA,
        EXPORT_SECURE_VALUE,
        IMPORT_SECURE_VALUE
    };
    // Base path to import/export to.
    constexpr std::u16string_view SECURE_VALUE_BASE_PATH = u"sdmc:/JKSM/Secure Values";
} // namespace

// Struct used to send data to functions that require confirmations.
typedef struct
{
        // Pointer to data of target title.
        const Data::TitleData *TargetTitle;
        // Save data type.
        Data::SaveDataType SaveType;
        // Pointer to current state to push messages.
        TitleOptionState *CreatingState;
} TargetStruct;

// Declarations of functions this state calls
static void EraseSaveData(System::Task *Task, std::shared_ptr<TargetStruct> DataStruct);
static void DeleteExtraData(System::Task *Task, std::shared_ptr<TargetStruct> DataStruct);

TitleOptionState::TitleOptionState(AppState *CreatingState, const Data::TitleData *TargetTitle, Data::SaveDataType SaveType)
    : m_CreatingState(CreatingState), m_TargetTitle(TargetTitle), m_SaveType(SaveType), m_OptionsMenu(70, 30, 258, 11)
{
    int CurrentString = 0;
    const char *MenuString = nullptr;
    while ((MenuString = Strings::GetStringByName(Strings::Names::TitleOptions, CurrentString++)) != nullptr)
    {
        m_OptionsMenu.AddOption(MenuString);
    }
}

void TitleOptionState::Update(void)
{
    m_OptionsMenu.Update();

    if (Input::ButtonPressed(KEY_A))
    {
        switch (m_OptionsMenu.GetSelected())
        {
            case DELETE_SECURE_VALUE:
            {
                if (!FS::DeleteSecureValue(m_TargetTitle->GetUniqueID()))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 8));
                }
                else
                {
                    char UTF8Title[0x40] = {0};
                    StringUtil::ToUTF8(m_TargetTitle->GetTitle(), UTF8Title, 0x40);

                    std::string SuccessMessage =
                        StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 7), UTF8Title);
                    ShowMessage(this, SuccessMessage);
                }
            }
            break;

            case ERASE_SAVE_DATA:
            {
                // Data struct
                std::shared_ptr<TargetStruct> DataStruct(new TargetStruct);
                DataStruct->TargetTitle = m_TargetTitle;
                DataStruct->SaveType = m_SaveType;
                DataStruct->CreatingState = this;

                // Warning/confirmation string.
                char UTF8Title[0x40] = {0};
                StringUtil::ToUTF8(m_TargetTitle->GetTitle(), UTF8Title, 0x40);
                std::string ConfirmErase =
                    StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionConfirmations, 0), UTF8Title);

                // This confirmation always requires holding so people can't blame me for them nuking their save data.
                JKSM::PushState(
                    std::make_shared<ConfirmState<System::Task, TaskState, TargetStruct>>(this, ConfirmErase, true, EraseSaveData, DataStruct));
            }
            break;

            case EXPORT_SECURE_VALUE:
            {
                uint64_t SecureValue = 0;
                if (!FsLib::GetSecureValueForTitle(m_TargetTitle->GetUniqueID(), SecureValue))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 1));
                    Logger::Log("Error getting secure value: %s", FsLib::GetErrorString());
                    return;
                }

                // Path to export to.
                FsLib::Path SecureValuePath = FsLib::Path(SECURE_VALUE_BASE_PATH) / m_TargetTitle->GetPathSafeTitle() + u".bin";

                // Open file.
                FsLib::File SecureValueFile(SecureValuePath, FS_OPEN_CREATE | FS_OPEN_WRITE);
                if (!SecureValueFile.IsOpen())
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 1));
                    Logger::Log("Error opening secure value output file: %s", FsLib::GetErrorString());
                    return;
                }

                // Try to write value
                if (SecureValueFile.Write(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 1));
                    Logger::Log("Error writing secure value to file: %s", FsLib::GetErrorString());
                    return;
                }

                // Need UTF-8 path
                char UTF8Path[FsLib::MAX_PATH] = {0};
                StringUtil::ToUTF8(SecureValuePath.CString(), UTF8Path, FsLib::MAX_PATH);
                // Message string.
                std::string SuccessMessage =
                    StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 0), UTF8Path);

                ShowMessage(this, SuccessMessage);
            }
            break;

            case IMPORT_SECURE_VALUE:
            {
                // Start with path.
                FsLib::Path SecureValuePath = FsLib::Path(SECURE_VALUE_BASE_PATH) / m_TargetTitle->GetPathSafeTitle() + u".bin";

                // Check if file even exists before going further.
                if (!FsLib::FileExists(SecureValuePath))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error: no secure value file found for title.");
                    return;
                }

                FsLib::File SecureValueFile(SecureValuePath, FS_OPEN_READ);
                if (!SecureValueFile.IsOpen())
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error opening secure value file for reading: %s", FsLib::GetErrorString());
                    return;
                }

                // Read value from file.
                uint64_t SecureValue = 0;
                if (SecureValueFile.Read(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error reading secure value from file: %s", FsLib::GetErrorString());
                    return;
                }

                // Set it.
                if (!FsLib::SetSecureValueForTitle(m_TargetTitle->GetUniqueID(), SecureValue))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error setting secure value: %s", FsLib::GetErrorString());
                    return;
                }

                // Get UTF-8 path for printing
                char UTF8Path[FsLib::MAX_PATH] = {0};
                StringUtil::ToUTF8(SecureValuePath.CString(), UTF8Path, FsLib::MAX_PATH);

                // Get message string.
                std::string SuccessMessage =
                    StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 2), UTF8Path);

                // Wew
                ShowMessage(this, SuccessMessage);
            }
            break;
        }
    }
    else if (Input::ButtonPressed(KEY_B))
    {
        AppState::Deactivate();
    }
}

void TitleOptionState::DrawTop(SDL_Surface *Target)
{
    if (m_CreatingState)
    {
        m_CreatingState->DrawTop(Target);
    }
    // Render dialog & menu
    UI::DrawDialogBox(Target, 48, 18, 304, 204);
    m_OptionsMenu.Draw(Target);
}

void TitleOptionState::DrawBottom(SDL_Surface *Target)
{
    if (m_CreatingState)
    {
        m_CreatingState->DrawBottom(Target);
    }
}

static void EraseSaveData(System::Task *Task, std::shared_ptr<TargetStruct> DataStruct)
{
    // Need UTF-8 encoded version of title for strings.
    char UTF8Title[0x40] = {0};
    StringUtil::ToUTF8(DataStruct->TargetTitle->GetTitle(), UTF8Title, 0x40);

    // Set task's status.
    Task->SetStatus(Strings::GetStringByName(Strings::Names::TitleOptionTaskStatus, 0), UTF8Title);

    // Attempt to open archive according to save type.
    if (DataStruct->SaveType == Data::SaveDataType::SaveTypeUser && !FsLib::OpenUserSaveData(FS::SAVE_MOUNT,
                                                                                             DataStruct->TargetTitle->GetMediaType(),
                                                                                             DataStruct->TargetTitle->GetLowerID(),
                                                                                             DataStruct->TargetTitle->GetUpperID()))
    {
        // Failure. Display message so user doesn't complain on github without reading the log.
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error occurred opening save for erasure: %s", FsLib::GetErrorString());
        Task->Finish();
        return;
    }
    else if (DataStruct->SaveType == Data::SaveDataType::SaveTypeSystem &&
             !FsLib::OpenSystemSaveData(FS::SAVE_MOUNT, DataStruct->TargetTitle->GetUniqueID()))
    {
        // Show message and bail
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error occurred opening system save for erasure: %s", FsLib::GetErrorString());
        Task->Finish();
        return;
    }

    // These are logged and messaged, but no bail.
    if (!FsLib::DeleteDirectoryRecursively(FS::SAVE_ROOT))
    {
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error erasing root of save data: %s", FsLib::GetErrorString());
    }

    if (!FsLib::ControlDevice(FS::SAVE_MOUNT))
    {
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error committing changes to save data: %s", FsLib::GetErrorString());
    }

    // Attempt to close archive.
    if (!FsLib::CloseDevice(FS::SAVE_MOUNT))
    {
        Logger::Log("Error closing device after save erasure: %s", FsLib::GetErrorString());
    }

    // Show success message
    std::string Success = StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 6), UTF8Title);
    ShowMessage(DataStruct->CreatingState, Success);

    // Should be done
    Task->Finish();
}

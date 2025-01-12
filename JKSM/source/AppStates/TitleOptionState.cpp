#include "AppStates/TitleOptionState.hpp"
#include "AppStates/ConfirmState.hpp"
#include "AppStates/MessageState.hpp"
#include "FS/FS.hpp"
#include "FS/SaveMount.hpp"
#include "Input.hpp"
#include "JKSM.hpp"
#include "StringUtil.hpp"
#include "System/Task.hpp"
#include "UI/Draw.hpp"
#include "UI/Strings.hpp"

namespace
{
    enum
    {
        DELETE_SECURE_VALUE,
        ERASE_SAVE_DATA,
        EXPORT_SECURE_VALUE,
        IMPORT_SECURE_VALUE
    };
}

// Struct to pass data to functions after confirmation.
typedef struct
{
        const Data::TitleData *TargetTitle;
} TargetStruct;

// Declarations for functions.
static void EraseSaveData(System::Task *Task, std::shared_ptr<TargetStruct> DataStruct);

TitleOptionState::TitleOptionState(AppState *CreatingState, const Data::TitleData *TargetTitle)
    : m_CreatingState(CreatingState), m_TargetTitle(TargetTitle), m_OptionsMenu(70, 30, 258, 11)
{
    int CurrentString = 0;
    const char *MenuString = nullptr;
    while ((MenuString = UI::Strings::GetStringByName(UI::Strings::Names::TitleOptions, CurrentString++)) != nullptr)
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
                FS::DeleteSecureValue(m_TargetTitle->GetUniqueID());
            }
            break;

            case ERASE_SAVE_DATA:
            {
                // Data to pass.
                std::shared_ptr<TargetStruct> DataStruct(new TargetStruct);
                DataStruct->TargetTitle = m_TargetTitle;

                // Warning/confirmation string.
                char UTF8Title[0x40] = {0};
                StringUtil::ToUTF8(m_TargetTitle->GetTitle(), UTF8Title, 0x40);
                std::string ConfirmErase =
                    StringUtil::GetFormattedString(UI::Strings::GetStringByName(UI::Strings::Names::TitleOptionConfirmations, 0), UTF8Title);

                // This confirmation always requires holding so people can't blame me for them nuking their save data.
                JKSM::PushState(
                    std::make_shared<ConfirmState<System::Task, TaskState, TargetStruct>>(this, ConfirmErase, true, EraseSaveData, DataStruct));
            }
            break;

            case EXPORT_SECURE_VALUE:
            {
                // Get/check if secure value actually exists for title. Not sure what title variation is at the moment.
                bool ValueExists = false;
                uint64_t SecureValue = 0;
                Result FsError =
                    FSUSER_GetSaveDataSecureValue(&ValueExists, &SecureValue, SECUREVALUE_SLOT_SD, m_TargetTitle->GetUniqueID(), 0);
                if (R_FAILED(FsError) || !ValueExists)
                {
                    Logger::Log("Error exporting secure value: doesn't exist or 0x%08X.", FsError);
                    return;
                }

                // Path to export to.
                FsLib::Path SecureValuePath =
                    FS::GetBasePath(Data::SaveDataType::SaveTypeUser) / m_TargetTitle->GetPathSafeTitle() / u".SecureValue.bin";

                // Open file
                FsLib::File SecureValueFile{SecureValuePath, FS_OPEN_CREATE | FS_OPEN_WRITE, sizeof(uint64_t)};
                if (!SecureValueFile.IsOpen())
                {
                    Logger::Log("Error exporting secure value: %s", FsLib::GetErrorString());
                    return;
                }

                // Write value
                SecureValueFile.Write(&SecureValue, sizeof(uint64_t));

                // Show message
                char UTF8Path[FsLib::MAX_PATH] = {0};
                StringUtil::ToUTF8(SecureValuePath.CString(), UTF8Path, FsLib::MAX_PATH);
                std::string Message =
                    StringUtil::GetFormattedString(UI::Strings::GetStringByName(UI::Strings::Names::TitleOptionMessages, 0), UTF8Path);

                JKSM::PushState(std::make_shared<MessageState>(this, Message));
            }
            break;

            case IMPORT_SECURE_VALUE:
            {
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
    // To do: Finish this for all save types. Not sure if this will work for extdata etc.
    if (!FsLib::OpenUserSaveData(FS::SAVE_MOUNT,
                                 DataStruct->TargetTitle->GetMediaType(),
                                 DataStruct->TargetTitle->GetLowerID(),
                                 DataStruct->TargetTitle->GetUpperID()))
    {
        Logger::Log("Error erasing save data: %s", FsLib::GetErrorString());
        Task->Finish();
        return;
    }

    // Update task.
    char UTF8Title[0x40] = {0};
    StringUtil::ToUTF8(DataStruct->TargetTitle->GetTitle(), UTF8Title, 0x40);
    Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::TitleOptionTaskStatus, 0), UTF8Title);

    if (!FsLib::DeleteDirectoryRecursively(FS::SAVE_ROOT))
    {
        Logger::Log("Error erasing save data: %s", FsLib::GetErrorString());
    }

    // Commit changes.
    if (!FsLib::ControlDevice(FS::SAVE_MOUNT))
    {
        Logger::Log("Error erasing save data: %s", FsLib::GetErrorString());
    }

    // Close
    FsLib::CloseDevice(FS::SAVE_MOUNT);

    Task->Finish();
}

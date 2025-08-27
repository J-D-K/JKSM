#include "AppStates/TitleOptionState.hpp"

#include "AppStates/ConfirmState.hpp"
#include "AppStates/MessageState.hpp"
#include "FS/FS.hpp"
#include "FS/SaveMount.hpp"
#include "Input.hpp"
#include "JKSM.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"
#include "System/Task.hpp"
#include "UI/Draw.hpp"
#include "fslib.hpp"

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
    : m_CreatingState(CreatingState)
    , m_TargetTitle(TargetTitle)
    , m_SaveType(SaveType)
    , m_OptionsMenu(70, 30, 258, 11)
{
    int CurrentString      = 0;
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
                        StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 7),
                                                       UTF8Title);
                    ShowMessage(this, SuccessMessage);
                }
            }
            break;

            case ERASE_SAVE_DATA:
            {
                // Data struct
                std::shared_ptr<TargetStruct> DataStruct(new TargetStruct);
                DataStruct->TargetTitle   = m_TargetTitle;
                DataStruct->SaveType      = m_SaveType;
                DataStruct->CreatingState = this;

                // Warning/confirmation string.
                char UTF8Title[0x40] = {0};
                StringUtil::ToUTF8(m_TargetTitle->GetTitle(), UTF8Title, 0x40);
                std::string ConfirmErase =
                    StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionConfirmations, 0),
                                                   UTF8Title);

                // This confirmation always requires holding so people can't blame me for them nuking their save data.
                JKSM::PushState(std::make_shared<ConfirmState<System::Task, TaskState, TargetStruct>>(this,
                                                                                                      ConfirmErase,
                                                                                                      true,
                                                                                                      EraseSaveData,
                                                                                                      DataStruct));
            }
            break;

            case EXPORT_SECURE_VALUE:
            {
                uint64_t SecureValue = 0;
                if (!fslib::get_secure_value_for_title(m_TargetTitle->GetUniqueID(), SecureValue))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 1));
                    Logger::Log("Error getting secure value: %s", fslib::error::get_string());
                    return;
                }

                // Path to export to.
                fslib::Path SecureValuePath = fslib::Path(SECURE_VALUE_BASE_PATH) / m_TargetTitle->GetPathSafeTitle() + u".bin";

                // Open file.
                fslib::File SecureValueFile(SecureValuePath, FS_OPEN_CREATE | FS_OPEN_WRITE);
                if (!SecureValueFile.is_open())
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 1));
                    Logger::Log("Error opening secure value output file: %s", fslib::error::get_string());
                    return;
                }

                // Try to write value
                if (SecureValueFile.write(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 1));
                    Logger::Log("Error writing secure value to file: %s", fslib::error::get_string());
                    return;
                }

                // Need UTF-8 path
                char UTF8Path[fslib::MAX_PATH] = {0};
                StringUtil::ToUTF8(SecureValuePath.full_path(), UTF8Path, fslib::MAX_PATH);
                // Message string.
                std::string SuccessMessage =
                    StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 0), UTF8Path);

                ShowMessage(this, SuccessMessage);
            }
            break;

            case IMPORT_SECURE_VALUE:
            {
                // Start with path.
                fslib::Path SecureValuePath = fslib::Path(SECURE_VALUE_BASE_PATH) / m_TargetTitle->GetPathSafeTitle() + u".bin";

                // Check if file even exists before going further.
                if (!fslib::file_exists(SecureValuePath))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error: no secure value file found for title.");
                    return;
                }

                fslib::File SecureValueFile(SecureValuePath, FS_OPEN_READ);
                if (!SecureValueFile.is_open())
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error opening secure value file for reading: %s", fslib::error::get_string());
                    return;
                }

                // Read value from file.
                uint64_t SecureValue = 0;
                if (SecureValueFile.read(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error reading secure value from file: %s", fslib::error::get_string());
                    return;
                }

                // Set it.
                if (!fslib::set_secure_value_for_title(m_TargetTitle->GetUniqueID(), SecureValue))
                {
                    ShowMessage(this, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 3));
                    Logger::Log("Error setting secure value: %s", fslib::error::get_string());
                    return;
                }

                // Get UTF-8 path for printing
                char UTF8Path[fslib::MAX_PATH] = {0};
                StringUtil::ToUTF8(SecureValuePath.full_path(), UTF8Path, fslib::MAX_PATH);

                // Get message string.
                std::string SuccessMessage =
                    StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 2), UTF8Path);

                // Wew
                ShowMessage(this, SuccessMessage);
            }
            break;
        }
    }
    else if (Input::ButtonPressed(KEY_B)) { AppState::Deactivate(); }
}

void TitleOptionState::DrawTop(SDL_Surface *Target)
{
    if (m_CreatingState) { m_CreatingState->DrawTop(Target); }
    // Render dialog & menu
    UI::DrawDialogBox(Target, 48, 18, 304, 204);
    m_OptionsMenu.Draw(Target);
}

void TitleOptionState::DrawBottom(SDL_Surface *Target)
{
    if (m_CreatingState) { m_CreatingState->DrawBottom(Target); }
}

static void EraseSaveData(System::Task *Task, std::shared_ptr<TargetStruct> DataStruct)
{
    // Need UTF-8 encoded version of title for strings.
    char UTF8Title[0x40] = {0};
    StringUtil::ToUTF8(DataStruct->TargetTitle->GetTitle(), UTF8Title, 0x40);

    // Set task's status.
    Task->SetStatus(Strings::GetStringByName(Strings::Names::TitleOptionTaskStatus, 0), UTF8Title);

    // Attempt to open archive according to save type.
    if (DataStruct->SaveType == Data::SaveDataType::SaveTypeUser &&
        !fslib::open_user_save_data(FS::SAVE_MOUNT,
                                    DataStruct->TargetTitle->GetMediaType(),
                                    DataStruct->TargetTitle->GetTitleID()))
    {
        // Failure. Display message so user doesn't complain on github without reading the log.
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error occurred opening save for erasure: %s", fslib::error::get_string());
        Task->Finish();
        return;
    }
    else if (DataStruct->SaveType == Data::SaveDataType::SaveTypeSystem &&
             !fslib::open_system_save_data(FS::SAVE_MOUNT, DataStruct->TargetTitle->GetUniqueID()))
    {
        // Show message and bail
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error occurred opening system save for erasure: %s", fslib::error::get_string());
        Task->Finish();
        return;
    }

    // These are logged and messaged, but no bail.
    if (!fslib::delete_directory_recursively(FS::SAVE_ROOT))
    {
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error erasing root of save data: %s", fslib::error::get_string());
    }

    if (!fslib::control_device(FS::SAVE_MOUNT))
    {
        ShowMessage(DataStruct->CreatingState, Strings::GetStringByName(Strings::Names::TitleOptionMessages, 4));
        Logger::Log("Error committing changes to save data: %s", fslib::error::get_string());
    }

    // Attempt to close archive.
    if (!fslib::close_device(FS::SAVE_MOUNT))
    {
        Logger::Log("Error closing device after save erasure: %s", fslib::error::get_string());
    }

    // Show success message
    std::string Success =
        StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::TitleOptionMessages, 6), UTF8Title);
    ShowMessage(DataStruct->CreatingState, Success);

    // Should be done
    Task->Finish();
}

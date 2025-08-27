#include "AppStates/BackupMenuState.hpp"

#include "AppStates/ConfirmState.hpp"
#include "AppStates/ProgressTaskState.hpp"
#include "AppStates/TaskState.hpp"
#include "Config.hpp"
#include "Data/Data.hpp"
#include "FS/FS.hpp"
#include "FS/IO.hpp"
#include "FS/SaveMount.hpp"
#include "Input.hpp"
#include "JKSM.hpp"
#include "Keyboard.hpp"
#include "SDL/SDL.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"
#include "System/ProgressTask.hpp"
#include "System/Task.hpp"

#include <string_view>

// Struct used for confirming actions. This is a general struct and not everything is used by every action/function
typedef struct
{
        fslib::Path TargetPath;
        Data::SaveDataType SaveType;
        const Data::TitleData *TargetTitle;
        BackupMenuState *CallingState = nullptr;
} TargetStruct;

// Declarations. Definitions after class members.
static void CreateNewBackup(System::ProgressTask *Task,
                            fslib::Path BackupPath,
                            const Data::TitleData *TargetTitle,
                            BackupMenuState *CreatingState);
static void OverwriteBackup(System::ProgressTask *Task, std::shared_ptr<TargetStruct> DataStruct);
static void RestoreBackup(System::ProgressTask *Task, std::shared_ptr<TargetStruct> DataStruct);
static void DeleteBackup(System::Task *Task, std::shared_ptr<TargetStruct> DataStruct);

// We're going to mark this as a task even though it isn't so JKSM doesn't let users shift from it.
BackupMenuState::BackupMenuState(AppState *CreatingState, const Data::TitleData *Data, Data::SaveDataType SaveType)
    : AppState(AppState::StateFlags::SemiLock)
    , m_CreatingState(CreatingState)
    , m_Data(Data)
    , m_SaveType(SaveType)
    , m_BackupMenu(4, 20, 312, 12)
{
    m_DirectoryPath = FS::GetBasePath(SaveType) / Data->GetPathSafeTitle();
    if (!fslib::directory_exists(m_DirectoryPath) && !fslib::create_directory(m_DirectoryPath))
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_DirectoryPath.full_path(), UTF8Buffer, 0x80);
        Logger::Log("Something went wrong creating %s for %016llX: %s",
                    UTF8Buffer,
                    Data->GetTitleID(),
                    fslib::error::get_string());
    }
    // Center text
    m_TextX = 160 - (m_Noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::BackupMenuCurrentBackups, 0)) / 2);

    BackupMenuState::Refresh();
}

BackupMenuState::~BackupMenuState()
{
    if (!fslib::close_device(FS::SAVE_MOUNT)) { Logger::Log("Error closing save archive: %s", fslib::error::get_string()); }
}

void BackupMenuState::Update(void)
{
    // Scoped lock for updating the actual menu.
    {
        std::scoped_lock<std::mutex> ListingLock(m_ListingMutex);
        m_BackupMenu.Update();
    }

    // New Backup
    if (Input::ButtonPressed(KEY_A) && m_BackupMenu.GetSelected() == 0)
    {
        // Buffer for actual backup name.
        char16_t BackupName[0x40] = {0};

        // Default text for keyboard. To do: This is kind of pointless if a shortcut is used, but I don't like nested ifs to
        // begin with.
        char DefaultText[0x40] = {0};
        StringUtil::GetDateTimeString(DefaultText, 0x40, StringUtil::DateFormats::DATE_FMT_YMD);

        // Shortcut checks
        if (Input::ButtonHeld(KEY_R))
        {
            StringUtil::GetDateTimeString(BackupName, 0x40, StringUtil::DateFormats::DATE_FMT_YMD);
        }
        else if (Input::ButtonHeld(KEY_L))
        {
            StringUtil::GetDateTimeString(BackupName, 0x40, StringUtil::DateFormats::DATE_FMT_YDM);
        }
        else if (!Keyboard::GetStringWithKeyboard(DefaultText, BackupName, 0x40))
        {
            // If this fails, bail from this function before continuing.
            return;
        }

        // Create path
        fslib::Path BackupPath = m_DirectoryPath / BackupName;

        JKSM::PushState(std::make_shared<ProgressTaskState>(this, CreateNewBackup, BackupPath, m_Data, this));
    }
    else if (Input::ButtonPressed(KEY_A) && m_BackupMenu.GetSelected() > 0)
    {
        // Confirm struct
        std::shared_ptr<TargetStruct> DataStruct(new TargetStruct);
        DataStruct->TargetPath = m_DirectoryPath / m_DirectoryListing[m_BackupMenu.GetSelected() - 1];

        // Query string
        char TargetName[fslib::MAX_PATH] = {0};
        StringUtil::ToUTF8(m_DirectoryListing[m_BackupMenu.GetSelected() - 1].get_filename(), TargetName, fslib::MAX_PATH);
        std::string ConfirmOverwrite =
            StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::BackupMenuConfirmations, 0), TargetName);

        JKSM::PushState(std::make_shared<ConfirmState<System::ProgressTask, ProgressTaskState, TargetStruct>>(
            this,
            ConfirmOverwrite,
            Config::GetByKey(Config::Keys::HoldToOverwrite),
            OverwriteBackup,
            DataStruct));
    }
    else if (Input::ButtonPressed(KEY_Y) && m_BackupMenu.GetSelected() > 0)
    {
        // Create confirmation struct.
        std::shared_ptr<TargetStruct> ConfirmStruct(new TargetStruct);
        ConfirmStruct->TargetPath  = m_DirectoryPath / m_DirectoryListing[m_BackupMenu.GetSelected() - 1];
        ConfirmStruct->SaveType    = m_SaveType;
        ConfirmStruct->TargetTitle = m_Data;

        // Query string
        char TargetName[fslib::MAX_PATH] = {0};
        StringUtil::ToUTF8(m_DirectoryListing[m_BackupMenu.GetSelected() - 1].get_filename(), TargetName, fslib::MAX_PATH);
        std::string RestoreString =
            StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::BackupMenuConfirmations, 1), TargetName);

        // Create  & push new confirmation.
        JKSM::PushState(std::make_shared<ConfirmState<System::ProgressTask, ProgressTaskState, TargetStruct>>(
            this,
            RestoreString,
            Config::GetByKey(Config::Keys::HoldToRestore),
            RestoreBackup,
            ConfirmStruct));
    }
    else if (Input::ButtonPressed(KEY_X) && m_BackupMenu.GetSelected() > 0)
    {
        // Confirm struct
        std::shared_ptr<TargetStruct> ConfirmStruct(new TargetStruct);
        ConfirmStruct->TargetPath   = m_DirectoryPath / m_DirectoryListing[m_BackupMenu.GetSelected() - 1];
        ConfirmStruct->CallingState = this;

        // String
        char TargetName[fslib::MAX_PATH] = {0};
        StringUtil::ToUTF8(m_DirectoryListing[m_BackupMenu.GetSelected() - 1].get_filename(), TargetName, fslib::MAX_PATH);
        std::string DeleteString =
            StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::BackupMenuConfirmations, 2), TargetName);

        JKSM::PushState(
            std::make_shared<ConfirmState<System::Task, TaskState, TargetStruct>>(this,
                                                                                  DeleteString,
                                                                                  Config::GetByKey(Config::Keys::HoldToDelete),
                                                                                  DeleteBackup,
                                                                                  ConfirmStruct));
    }
    else if (Input::ButtonPressed(KEY_B)) { AppState::Deactivate(); }
}

void BackupMenuState::DrawTop(SDL_Surface *Target) { m_CreatingState->DrawTop(Target); }

void BackupMenuState::DrawBottom(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target,
                       m_TextX,
                       1,
                       12,
                       m_Noto->NO_TEXT_WRAP,
                       Strings::GetStringByName(Strings::Names::BackupMenuCurrentBackups, 0));

    {
        std::scoped_lock<std::mutex> ListingLock(m_ListingMutex);
        m_BackupMenu.Draw(Target);
    }
}

void BackupMenuState::Refresh(void)
{
    std::scoped_lock<std::mutex> ListingLock(m_ListingMutex);
    m_DirectoryListing.open(m_DirectoryPath);
    m_BackupMenu.Reset();

    // Loop and copy directory to menu after adding new
    m_BackupMenu.AddOption(Strings::GetStringByName(Strings::Names::FolderMenuNew, 0));
    for (uint32_t i = 0; i < m_DirectoryListing.get_count(); i++)
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_DirectoryListing[i].get_filename(), UTF8Buffer, 0x80);
        m_BackupMenu.AddOption(UTF8Buffer);
    }
}

static void CreateNewBackup(System::ProgressTask *Task,
                            fslib::Path BackupPath,
                            const Data::TitleData *TargetTitle,
                            BackupMenuState *CreatingState)
{
    // Make sure destination exists.
    if (!Config::GetByKey(Config::Keys::ExportToZip) && BackupPath.get_extension() != u"zip" &&
        (fslib::directory_exists(BackupPath) || fslib::create_directory(BackupPath)))
    {
        // Copy save as-is to target directory.
        FS::CopyDirectoryToDirectory(Task, FS::SAVE_ROOT, BackupPath, false);

        // If secure value preservation is active, try to dump it with the save if it exists.
        uint64_t SecureValue = 0;
        if (Config::GetByKey(Config::Keys::PreserveSecureValues) &&
            fslib::get_secure_value_for_title(TargetTitle->GetUniqueID(), SecureValue))
        {
            // Path. Trying to make sure no game would possibly use this...
            fslib::Path SecureValuePath = BackupPath / u"._secure_value";

            fslib::File SecureValueFile(SecureValuePath, FS_OPEN_CREATE | FS_OPEN_WRITE);
            if (!SecureValueFile.is_open() || SecureValueFile.write(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t))
            {
                Logger::Log("Error while exporting secure value during backup: %s", fslib::error::get_string());
                return;
            }
        }
    }
    else if (Config::GetByKey(Config::Keys::ExportToZip) || BackupPath.get_extension() == u"zip")
    {
        // Create the main backup first.
        zipFile Backup = zipOpen("sdmc:/Temp.zip", APPEND_STATUS_CREATE);
        FS::CopyDirectoryToZip(Task, FS::SAVE_ROOT, Backup);

        // Check if we need to add the secure value to the zip.
        uint64_t SecureValue = 0;
        if (Config::GetByKey(Config::Keys::PreserveSecureValues) &&
            fslib::get_secure_value_for_title(TargetTitle->GetUniqueID(), SecureValue) &&
            zipOpenNewFileInZip(Backup, "._secure_value", NULL, 0, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) ==
                ZIP_OK)
        {
            zipWriteInFileInZip(Backup, &SecureValue, sizeof(uint64_t));
            zipCloseFileInZip(Backup);
        }

        // Close and move zip to destination.
        zipClose(Backup, NULL);
        fslib::rename_file(u"sdmc:/Temp.zip", BackupPath);
    }
    CreatingState->Refresh();
    Task->Finish();
}

static void OverwriteBackup(System::ProgressTask *Task, std::shared_ptr<TargetStruct> DataStruct)
{
    // fslib::directory_exists can also be used to test if the target is a directory.
    if (fslib::directory_exists(DataStruct->TargetPath) && fslib::delete_directory_recursively(DataStruct->TargetPath) &&
        fslib::create_directory(DataStruct->TargetPath))
    {
        FS::CopyDirectoryToDirectory(Task, FS::SAVE_ROOT, DataStruct->TargetPath, false);
    }
    else if (fslib::file_exists(DataStruct->TargetPath) && fslib::delete_file(DataStruct->TargetPath))
    {
        // Assuming this is a zip.
        zipFile Backup = zipOpen("sdmc:/Temp.zip", APPEND_STATUS_CREATE);
        FS::CopyDirectoryToZip(Task, FS::SAVE_ROOT, Backup);
        zipClose(Backup, NULL);
        fslib::rename_file(u"sdmc:/Temp.zip", DataStruct->TargetPath);
    }
    Task->Finish();
}

static void RestoreBackup(System::ProgressTask *Task, std::shared_ptr<TargetStruct> DataStruct)
{
    if (!fslib::delete_directory_recursively(FS::SAVE_ROOT))
    {
        Logger::Log("Error occurred resetting save data: %s", fslib::error::get_string());
        Task->Finish();
        return;
    }

    // Secure value check. Scoped so path gets freed immediately after check is finished.
    {
        // This is the path the secure value if it exits.
        fslib::Path SecureValuePath = DataStruct->TargetPath / u"._secure_value";
        if (Config::GetByKey(Config::Keys::PreserveSecureValues) && fslib::file_exists(SecureValuePath))
        {
            // Try to open file and read secure value.
            uint64_t SecureValue = 0;
            fslib::File SecureValueFile(SecureValuePath, FS_OPEN_READ);
            if (!SecureValueFile.is_open() || SecureValueFile.read(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t) ||
                !fslib::set_secure_value_for_title(DataStruct->TargetTitle->GetUniqueID(), SecureValue))
            {
                Logger::Log("Error occurred while attempting to set and preserve secure value for game.");
            }
        }
        else if (!Config::GetByKey(Config::Keys::PreserveSecureValues) &&
                 !FS::DeleteSecureValue(DataStruct->TargetTitle->GetUniqueID()))
        {
            Logger::Log("Error occurred while trying to delete secure value for game.");
        }
    }

    // Whether or not committing data is needed.
    bool CommitData = (DataStruct->SaveType == Data::SaveDataType::SaveTypeUser ||
                       DataStruct->SaveType == Data::SaveDataType::SaveTypeSystem);

    // This can also be used to test if the target is a directory. Not just if it exists.
    if (fslib::directory_exists(DataStruct->TargetPath))
    {
        FS::CopyDirectoryToDirectory(Task, DataStruct->TargetPath, FS::SAVE_ROOT, CommitData);
    }
    else
    {
        // This is just assuming the file is a zip file. This probably isn't the greatest idea.
        fslib::rename_file(DataStruct->TargetPath, u"sdmc:/Temp.zip");

        // Open file for *unzips* and extract it to save archive.
        unzFile UnZip = unzOpen("sdmc:/Temp.zip");
        FS::CopyZipToDirectory(Task, UnZip, FS::SAVE_ROOT, CommitData);
        unzClose(UnZip);

        // Rename file back to original.
        fslib::rename_file(u"sdmc:/Temp.zip", DataStruct->TargetPath);
    }
    Task->Finish();
}

static void DeleteBackup(System::Task *Task, std::shared_ptr<TargetStruct> DataStruct)
{
    // Set status just in case deletion takes a little while.
    char TargetName[fslib::MAX_PATH] = {0};
    StringUtil::ToUTF8(DataStruct->TargetPath.full_path(), TargetName, fslib::MAX_PATH);
    Task->SetStatus(Strings::GetStringByName(Strings::Names::DeletingBackup, 0), TargetName);

    if (fslib::directory_exists(DataStruct->TargetPath) && fslib::delete_directory_recursively(DataStruct->TargetPath))
    {
        DataStruct->CallingState->Refresh();
    }
    else if (fslib::file_exists(DataStruct->TargetPath) && fslib::delete_file(DataStruct->TargetPath))
    {
        DataStruct->CallingState->Refresh();
    }
    else { Logger::Log("Error deleting backup: %s", fslib::error::get_string()); }

    DataStruct->CallingState->Refresh();

    Task->Finish();
}

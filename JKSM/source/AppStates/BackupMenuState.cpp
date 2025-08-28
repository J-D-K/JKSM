#include "appstates/BackupMenuState.hpp"

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
#include "appstates/ConfirmState.hpp"
#include "appstates/ProgressTaskState.hpp"
#include "appstates/TaskState.hpp"

#include <string_view>

// Struct used for confirming actions. This is a general struct and not everything is used by every action/function
// clang-format off
typedef struct
{
    fslib::Path TargetPath;
    Data::SaveDataType SaveType;
    const Data::TitleData *TargetTitle;
    BackupMenuState *CallingState = nullptr;
} TargetStruct;
// clang-format on

// Declarations. Definitions after class members.
static void CreateNewBackup(System::ProgressTask *task,
                            fslib::Path backupPath,
                            const Data::TitleData *targetTitle,
                            BackupMenuState *creatingState);
static void OverwriteBackup(System::ProgressTask *task, std::shared_ptr<TargetStruct> dataStruct);
static void RestoreBackup(System::ProgressTask *task, std::shared_ptr<TargetStruct> dataStruct);
static void DeleteBackup(System::Task *task, std::shared_ptr<TargetStruct> dataStruct);

// We're going to mark this as a task even though it isn't so JKSM doesn't let users shift from it.
BackupMenuState::BackupMenuState(BaseState *creatingState, const Data::TitleData *data, Data::SaveDataType saveType)
    : BaseState(BaseState::StateFlags::SemiLock)
    , m_creatingState(creatingState)
    , m_data(data)
    , m_saveType(saveType)
    , m_backupMenu(4, 20, 312, 12)
{
    m_directoryPath = FS::GetBasePath(saveType) / data->GetPathSafeTitle();
    if (!fslib::directory_exists(m_directoryPath) && !fslib::create_directory(m_directoryPath))
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_directoryPath.full_path(), UTF8Buffer, 0x80);
        logger::log("Something went wrong creating %s for %016llX: %s",
                    UTF8Buffer,
                    data->GetTitleID(),
                    fslib::error::get_string());
    }
    // Center text
    m_textX = 160 - (m_noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::BackupMenuCurrentBackups, 0)) / 2);

    BackupMenuState::refresh();
}

BackupMenuState::~BackupMenuState()
{
    if (!fslib::close_device(FS::SAVE_MOUNT)) { logger::log("Error closing save archive: %s", fslib::error::get_string()); }
}

void BackupMenuState::update()
{
    // Scoped lock for updating the actual menu.
    {
        std::lock_guard listGuard{m_listingMutex};
        m_backupMenu.Update();
    }

    // New Backup
    if (Input::ButtonPressed(KEY_A) && m_backupMenu.GetSelected() == 0)
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
        fslib::Path BackupPath = m_directoryPath / BackupName;

        JKSM::PushState(std::make_shared<ProgressTaskState>(this, CreateNewBackup, BackupPath, m_data, this));
    }
    else if (Input::ButtonPressed(KEY_A) && m_backupMenu.GetSelected() > 0)
    {
        // Confirm struct
        std::shared_ptr<TargetStruct> DataStruct(new TargetStruct);
        DataStruct->TargetPath = m_directoryPath / m_directoryListing[m_backupMenu.GetSelected() - 1];

        // Query string
        char TargetName[fslib::MAX_PATH] = {0};
        StringUtil::ToUTF8(m_directoryListing[m_backupMenu.GetSelected() - 1].get_filename(), TargetName, fslib::MAX_PATH);
        std::string ConfirmOverwrite =
            StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::BackupMenuConfirmations, 0), TargetName);

        JKSM::PushState(std::make_shared<ConfirmState<System::ProgressTask, ProgressTaskState, TargetStruct>>(
            this,
            ConfirmOverwrite,
            Config::GetByKey(Config::Keys::HoldToOverwrite),
            OverwriteBackup,
            DataStruct));
    }
    else if (Input::ButtonPressed(KEY_Y) && m_backupMenu.GetSelected() > 0)
    {
        // Create confirmation struct.
        std::shared_ptr<TargetStruct> ConfirmStruct(new TargetStruct);
        ConfirmStruct->TargetPath  = m_directoryPath / m_directoryListing[m_backupMenu.GetSelected() - 1];
        ConfirmStruct->SaveType    = m_saveType;
        ConfirmStruct->TargetTitle = m_data;

        // Query string
        char TargetName[fslib::MAX_PATH] = {0};
        StringUtil::ToUTF8(m_directoryListing[m_backupMenu.GetSelected() - 1].get_filename(), TargetName, fslib::MAX_PATH);
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
    else if (Input::ButtonPressed(KEY_X) && m_backupMenu.GetSelected() > 0)
    {
        // Confirm struct
        std::shared_ptr<TargetStruct> ConfirmStruct(new TargetStruct);
        ConfirmStruct->TargetPath   = m_directoryPath / m_directoryListing[m_backupMenu.GetSelected() - 1];
        ConfirmStruct->CallingState = this;

        // String
        char TargetName[fslib::MAX_PATH] = {0};
        StringUtil::ToUTF8(m_directoryListing[m_backupMenu.GetSelected() - 1].get_filename(), TargetName, fslib::MAX_PATH);
        std::string DeleteString =
            StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::BackupMenuConfirmations, 2), TargetName);

        JKSM::PushState(
            std::make_shared<ConfirmState<System::Task, TaskState, TargetStruct>>(this,
                                                                                  DeleteString,
                                                                                  Config::GetByKey(Config::Keys::HoldToDelete),
                                                                                  DeleteBackup,
                                                                                  ConfirmStruct));
    }
    else if (Input::ButtonPressed(KEY_B)) { BaseState::deactivate(); }
}

void BackupMenuState::draw_top(SDL_Surface *target) { m_creatingState->draw_top(target); }

void BackupMenuState::draw_bottom(SDL_Surface *target)
{
    SDL::DrawRect(target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_noto->BlitTextAt(target,
                       m_textX,
                       1,
                       12,
                       m_noto->NO_TEXT_WRAP,
                       Strings::GetStringByName(Strings::Names::BackupMenuCurrentBackups, 0));

    {
        std::lock_guard listGuard{m_listingMutex};
        m_backupMenu.Draw(target);
    }
}

void BackupMenuState::refresh()
{
    std::lock_guard listGuard{m_listingMutex};
    m_directoryListing.open(m_directoryPath);
    m_backupMenu.Reset();

    // Loop and copy directory to menu after adding new
    m_backupMenu.AddOption(Strings::GetStringByName(Strings::Names::FolderMenuNew, 0));
    for (uint32_t i = 0; i < m_directoryListing.get_count(); i++)
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_directoryListing[i].get_filename(), UTF8Buffer, 0x80);
        m_backupMenu.AddOption(UTF8Buffer);
    }
}

static void CreateNewBackup(System::ProgressTask *task,
                            fslib::Path backupPath,
                            const Data::TitleData *targetTitle,
                            BackupMenuState *creatingState)
{
    // Make sure destination exists.
    if (!Config::GetByKey(Config::Keys::ExportToZip) && backupPath.get_extension() != u"zip" &&
        (fslib::directory_exists(backupPath) || fslib::create_directory(backupPath)))
    {
        // Copy save as-is to target directory.
        FS::CopyDirectoryToDirectory(task, FS::SAVE_ROOT, backupPath, false);

        // If secure value preservation is active, try to dump it with the save if it exists.
        uint64_t SecureValue = 0;
        if (Config::GetByKey(Config::Keys::PreserveSecureValues) &&
            fslib::get_secure_value_for_title(targetTitle->GetUniqueID(), SecureValue))
        {
            // Path. Trying to make sure no game would possibly use this...
            fslib::Path SecureValuePath = backupPath / u"._secure_value";

            fslib::File SecureValueFile(SecureValuePath, FS_OPEN_CREATE | FS_OPEN_WRITE);
            if (!SecureValueFile.is_open() || SecureValueFile.write(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t))
            {
                logger::log("Error while exporting secure value during backup: %s", fslib::error::get_string());
                return;
            }
        }
    }
    else if (Config::GetByKey(Config::Keys::ExportToZip) || backupPath.get_extension() == u"zip")
    {
        // Create the main backup first.
        zipFile Backup = zipOpen("sdmc:/Temp.zip", APPEND_STATUS_CREATE);
        FS::CopyDirectoryToZip(task, FS::SAVE_ROOT, Backup);

        // Check if we need to add the secure value to the zip.
        uint64_t SecureValue = 0;
        if (Config::GetByKey(Config::Keys::PreserveSecureValues) &&
            fslib::get_secure_value_for_title(targetTitle->GetUniqueID(), SecureValue) &&
            zipOpenNewFileInZip(Backup, "._secure_value", NULL, 0, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) ==
                ZIP_OK)
        {
            zipWriteInFileInZip(Backup, &SecureValue, sizeof(uint64_t));
            zipCloseFileInZip(Backup);
        }

        // Close and move zip to destination.
        zipClose(Backup, NULL);
        fslib::rename_file(u"sdmc:/Temp.zip", backupPath);
    }
    creatingState->refresh();
    task->Finish();
}

static void OverwriteBackup(System::ProgressTask *task, std::shared_ptr<TargetStruct> dataStruct)
{
    // fslib::directory_exists can also be used to test if the target is a directory.
    if (fslib::directory_exists(dataStruct->TargetPath) && fslib::delete_directory_recursively(dataStruct->TargetPath) &&
        fslib::create_directory(dataStruct->TargetPath))
    {
        FS::CopyDirectoryToDirectory(task, FS::SAVE_ROOT, dataStruct->TargetPath, false);
    }
    else if (fslib::file_exists(dataStruct->TargetPath) && fslib::delete_file(dataStruct->TargetPath))
    {
        // Assuming this is a zip.
        zipFile Backup = zipOpen("sdmc:/Temp.zip", APPEND_STATUS_CREATE);
        FS::CopyDirectoryToZip(task, FS::SAVE_ROOT, Backup);
        zipClose(Backup, NULL);
        fslib::rename_file(u"sdmc:/Temp.zip", dataStruct->TargetPath);
    }
    task->Finish();
}

static void RestoreBackup(System::ProgressTask *task, std::shared_ptr<TargetStruct> dataStruct)
{
    if (!fslib::delete_directory_recursively(FS::SAVE_ROOT))
    {
        logger::log("Error occurred resetting save data: %s", fslib::error::get_string());
        task->Finish();
        return;
    }

    // Secure value check. Scoped so path gets freed immediately after check is finished.
    {
        // This is the path the secure value if it exits.
        fslib::Path SecureValuePath = dataStruct->TargetPath / u"._secure_value";
        if (Config::GetByKey(Config::Keys::PreserveSecureValues) && fslib::file_exists(SecureValuePath))
        {
            // Try to open file and read secure value.
            uint64_t SecureValue = 0;
            fslib::File SecureValueFile(SecureValuePath, FS_OPEN_READ);
            if (!SecureValueFile.is_open() || SecureValueFile.read(&SecureValue, sizeof(uint64_t)) != sizeof(uint64_t) ||
                !fslib::set_secure_value_for_title(dataStruct->TargetTitle->GetUniqueID(), SecureValue))
            {
                logger::log("Error occurred while attempting to set and preserve secure value for game.");
            }
        }
        else if (!Config::GetByKey(Config::Keys::PreserveSecureValues) &&
                 !FS::DeleteSecureValue(dataStruct->TargetTitle->GetUniqueID()))
        {
            logger::log("Error occurred while trying to delete secure value for game.");
        }
    }

    // Whether or not committing data is needed.
    bool CommitData = (dataStruct->SaveType == Data::SaveDataType::SaveTypeUser ||
                       dataStruct->SaveType == Data::SaveDataType::SaveTypeSystem);

    // This can also be used to test if the target is a directory. Not just if it exists.
    if (fslib::directory_exists(dataStruct->TargetPath))
    {
        FS::CopyDirectoryToDirectory(task, dataStruct->TargetPath, FS::SAVE_ROOT, CommitData);
    }
    else
    {
        // This is just assuming the file is a zip file. This probably isn't the greatest idea.
        fslib::rename_file(dataStruct->TargetPath, u"sdmc:/Temp.zip");

        // Open file for *unzips* and extract it to save archive.
        unzFile UnZip = unzOpen("sdmc:/Temp.zip");
        FS::CopyZipToDirectory(task, UnZip, FS::SAVE_ROOT, CommitData);
        unzClose(UnZip);

        // Rename file back to original.
        fslib::rename_file(u"sdmc:/Temp.zip", dataStruct->TargetPath);
    }
    task->Finish();
}

static void DeleteBackup(System::Task *task, std::shared_ptr<TargetStruct> dataStruct)
{
    // Set status just in case deletion takes a little while.
    char TargetName[fslib::MAX_PATH] = {0};
    StringUtil::ToUTF8(dataStruct->TargetPath.full_path(), TargetName, fslib::MAX_PATH);
    task->SetStatus(Strings::GetStringByName(Strings::Names::DeletingBackup, 0), TargetName);

    if (fslib::directory_exists(dataStruct->TargetPath) && fslib::delete_directory_recursively(dataStruct->TargetPath))
    {
        dataStruct->CallingState->refresh();
    }
    else if (fslib::file_exists(dataStruct->TargetPath) && fslib::delete_file(dataStruct->TargetPath))
    {
        dataStruct->CallingState->refresh();
    }
    else { logger::log("Error deleting backup: %s", fslib::error::get_string()); }

    dataStruct->CallingState->refresh();

    task->Finish();
}

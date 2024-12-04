#include "AppStates/BackupMenuState.hpp"
#include "AppStates/ProgressTaskState.hpp"
#include "AppStates/TaskState.hpp"
#include "Config.hpp"
#include "FS/FS.hpp"
#include "FS/IO.hpp"
#include "FS/SaveMount.hpp"
#include "Input.hpp"
#include "JKSM.hpp"
#include "Keyboard.hpp"
#include "SDL/SDL.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"

namespace
{
    constexpr std::u16string_view SAVE_ROOT = u"save:/";
}

// This is the function called to create a backup. I don't feel like dealing with all the crap to pass a member method from the clss itself.
static void CreateNewBackup(System::ProgressTask *Task, FsLib::Path BackupPath, BackupMenuState *CreatingState)
{
    // Make sure destination exists.
    if (!Config::GetByKey(Config::Keys::ExportToZip) && (FsLib::DirectoryExists(BackupPath) || FsLib::CreateDirectory(BackupPath)))
    {
        FS::CopyDirectoryToDirectory(Task, SAVE_ROOT, BackupPath, false);
    }
    else if (Config::GetByKey(Config::Keys::ExportToZip) || BackupPath.GetExtension() == u"zip")
    {
        zipFile Backup = zipOpen("sdmc:/Temp.zip", APPEND_STATUS_CREATE);
        FS::CopyDirectoryToZip(Task, SAVE_ROOT, Backup);
        zipClose(Backup, NULL);
        FsLib::RenameFile(u"sdmc:/Temp.zip", BackupPath);
    }
    CreatingState->Refresh();
    Task->Finish();
}

static void OverwriteBackup(System::ProgressTask *Task, FsLib::Path BackupPath)
{
    // This should only be triggered if the backup is a directory to begin with. DirectoryExists should return false if it's a file.
    if (FsLib::DirectoryExists(BackupPath) && FsLib::DeleteDirectoryRecursively(BackupPath) && FsLib::CreateDirectory(BackupPath))
    {
        FS::CopyDirectoryToDirectory(Task, SAVE_ROOT, BackupPath, false);
    }
    else if (FsLib::FileExists(BackupPath) && FsLib::DeleteFile(BackupPath))
    {
        // We're going to assume this is a zip.
        zipFile Backup = zipOpen("sdmc:/Temp.zip", APPEND_STATUS_CREATE);
        FS::CopyDirectoryToZip(Task, SAVE_ROOT, Backup);
        zipClose(Backup, NULL);
        FsLib::RenameFile(u"sdmc:/Temp.zip", BackupPath);
    }
    Task->Finish();
}

static void RestoreBackup(System::ProgressTask *Task, FsLib::Path BackupPath, Data::SaveDataType SaveType)
{
    // To do: Figure out why this isn't working right, but only here.
    /*if (!FsLib::DeleteDirectoryRecursively(SAVE_ROOT))
    {
        Logger::Log("Error occurred resetting save data: %s", FsLib::GetErrorString());
        Task->Finish();
        return;
    }*/

    FS_Archive Archive;
    if (!FsLib::GetArchiveByDeviceName(SAVE_MOUNT, &Archive))
    {
        Logger::Log("Error getting archive: %s", FsLib::GetErrorString());
    }

    Result FsError = FSUSER_DeleteDirectoryRecursively(Archive, fsMakePath(PATH_ASCII, "/"));
    if (R_FAILED(FsError))
    {
        Logger::Log("Failed this way too!");
        Task->Finish();
        return;
    }

    bool CommitData = (SaveType == Data::SaveDataType::SaveTypeUser || SaveType == Data::SaveDataType::SaveTypeSystem) ? true : false;

    // This should be able to tell whether or not the backup is a directory
    if (FsLib::DirectoryExists(BackupPath))
    {
        FS::CopyDirectoryToDirectory(Task, BackupPath, SAVE_ROOT, CommitData);
    }
    else
    {
        // This isn't really needed, but I don't feel like converting path formats over and over. This is easier.
        FsLib::RenameFile(BackupPath, u"sdmc:/Temp.zip");

        unzFile UnZip = unzOpen("sdmc:/Temp.zip");
        // This is always assumed to be to save data,
        FS::CopyZipToDirectory(Task, UnZip, SAVE_ROOT, CommitData);
        unzClose(UnZip);
        FsLib::RenameFile(u"sdmc:/Temp.zip", BackupPath);
    }

    Task->Finish();
}

// This doesn't really need to be threaded, but whatever. At least it won't look like JKSM just froze for certain games.
static void DeleteBackup(System::Task *Task, FsLib::Path BackupPath, BackupMenuState *CreatingState)
{
    if (FsLib::DirectoryExists(BackupPath) && !FsLib::DeleteDirectoryRecursively(BackupPath))
    {
        Logger::Log("Error deleting backup: %s", FsLib::GetErrorString());
    }
    else if (FsLib::FileExists(BackupPath) && !FsLib::DeleteFile(BackupPath))
    {
        Logger::Log("Error deleting backup: %s", FsLib::GetErrorString());
    }
    CreatingState->Refresh();
    Task->Finish();
}

// We're going to mark this as a task even though it isn't so JKSM doesn't let users shift from it.
BackupMenuState::BackupMenuState(AppState *CreatingState, const Data::TitleData *Data, Data::SaveDataType SaveType)
    : AppState(AppState::StateFlags::Lock), m_CreatingState(CreatingState), m_Data(Data), m_SaveType(SaveType),
      m_BackupMenu(std::make_unique<UI::Menu>(4, 20, 312, 12))
{
    m_DirectoryPath = FS::GetBasePath(SaveType) / Data->GetPathSafeTitle();
    if (!FsLib::DirectoryExists(m_DirectoryPath) && !FsLib::CreateDirectory(m_DirectoryPath))
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_DirectoryPath.CString(), UTF8Buffer, 0x80);
        Logger::Log("Something went wrong creating %s for %016llX: %s", UTF8Buffer, Data->GetTitleID(), FsLib::GetErrorString());
    }
    // Center text
    m_TextX = 160 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI::Strings::Names::BackupMenuCurrentBackups, 0)) / 2);

    BackupMenuState::Refresh();
}

BackupMenuState::~BackupMenuState()
{
    FsLib::CloseDevice(SAVE_MOUNT);
}

void BackupMenuState::Update(void)
{
    {
        std::scoped_lock<std::mutex> ListingLock(m_ListingMutex);
        m_BackupMenu->Update();
    }
    // New Backup
    if (Input::ButtonPressed(KEY_A) && m_BackupMenu->GetSelected() == 0)
    {
        // Get name
        char16_t BackupName[0x40] = {0};
        if (!GetStringWithKeyboard(BackupName, 0x40))
        {
            return;
        }

        // Create path
        FsLib::Path BackupPath = m_DirectoryPath / BackupName;

        JKSM::PushState(std::make_shared<ProgressTaskState>(this, CreateNewBackup, BackupPath, this));
    }
    else if (Input::ButtonPressed(KEY_A) && m_BackupMenu->GetSelected() > 0)
    {
        // Selected needs to be offset by 1 to account for New
        FsLib::Path BackupPath = m_DirectoryPath / m_DirectoryListing[m_BackupMenu->GetSelected() - 1];

        // To do: Confirm
        JKSM::PushState(std::make_shared<ProgressTaskState>(this, OverwriteBackup, BackupPath));
    }
    else if (Input::ButtonPressed(KEY_Y) && m_BackupMenu->GetSelected() > 0)
    {
        FsLib::Path BackupPath = m_DirectoryPath / m_DirectoryListing[m_BackupMenu->GetSelected() - 1];

        JKSM::PushState(std::make_shared<ProgressTaskState>(this, RestoreBackup, BackupPath, m_SaveType));
    }
    else if (Input::ButtonPressed(KEY_X) && m_BackupMenu->GetSelected() > 0)
    {
        FsLib::Path TargetPath = m_DirectoryPath / m_DirectoryListing[m_BackupMenu->GetSelected() - 1];

        JKSM::PushState(std::make_shared<TaskState>(this, DeleteBackup, TargetPath, this));
    }
    else if (Input::ButtonPressed(KEY_B))
    {
        AppState::Deactivate();
    }
}

void BackupMenuState::DrawTop(SDL_Surface *Target)
{
    m_CreatingState->DrawTop(Target);
}

void BackupMenuState::DrawBottom(SDL_Surface *Target)
{
    SDL::DrawRect(Target, 0, 0, 320, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Target,
                       m_TextX,
                       1,
                       12,
                       m_Noto->NO_TEXT_WRAP,
                       UI::Strings::GetStringByName(UI::Strings::Names::BackupMenuCurrentBackups, 0));

    {
        std::scoped_lock<std::mutex> ListingLock(m_ListingMutex);
        m_BackupMenu->Draw(Target);
    }
}

void BackupMenuState::Refresh(void)
{
    std::scoped_lock<std::mutex> ListingLock(m_ListingMutex);
    m_DirectoryListing.Open(m_DirectoryPath);
    m_BackupMenu->Reset();

    // Loop and copy directory to menu after adding new
    m_BackupMenu->AddOption(UI::Strings::GetStringByName(UI::Strings::Names::FolderMenuNew, 0));
    for (uint32_t i = 0; i < m_DirectoryListing.GetEntryCount(); i++)
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_DirectoryListing[i], UTF8Buffer, 0x80);
        m_BackupMenu->AddOption(UTF8Buffer);
    }
}

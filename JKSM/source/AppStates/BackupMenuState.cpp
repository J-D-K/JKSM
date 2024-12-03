#include "AppStates/BackupMenuState.hpp"
#include "AppStates/ProgressTaskState.hpp"
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

// This is the function called to create a backup. I don't feel like dealing with all the crap to pass a member method from the clss itself.
static void CreateNewBackupFolder(System::ProgressTask *Task, FsLib::Path BackupPath, BackupMenuState *CreatingState)
{
    // Make sure destination exists.
    if (!FsLib::DirectoryExists(BackupPath) && !FsLib::CreateDirectory(BackupPath))
    {
        Logger::Log("Error creating backup directory: %s", FsLib::GetErrorString());
        Task->Finish();
        return;
    }
    FS::CopyDirectoryToDirectory(Task, u"save:/", BackupPath, false);
    CreatingState->Refresh();
    Task->Finish();
}

/*
    This needs to be passed the Zip, because default 3DS threads don't have near enough stack space for minizip. The zip is then renamed to
    the target file instead of needed to convert everything back an forth from UTF8 to UTF16 over and over.
*/
static void CreateNewBackupZip(System::ProgressTask *Task, zipFile ZipOut, FsLib::Path TargetDestination, BackupMenuState *CreatingState)
{
    if (!ZipOut)
    {
        Logger::Log("ZipOut is NULL!");
        Task->Finish();
        return;
    }
    // Copy the save to the zip.
    FS::CopyDirectoryToZip(Task, u"save:/", ZipOut);
    // Now that it's done close it.
    zipClose(ZipOut, NULL);
    // Try to rename it. We always know the original path.
    if (!FsLib::RenameFile(u"sdmc:/Temp.zip", TargetDestination))
    {
        Logger::Log("Error moving zip to destination: %s", FsLib::GetErrorString());
    }
    CreatingState->Refresh();
    Task->Finish();
}

// We're going to mark this as a task even though it isn't so JKSM doesn't let users shift from it.
BackupMenuState::BackupMenuState(AppState *CreatingState, const Data::TitleData *Data, Data::SaveDataType SaveType)
    : AppState(AppState::StateFlags::Lock), m_CreatingState(CreatingState), m_Data(Data),
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
    m_BackupMenu->Update();

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

        if (Config::GetByKey(Config::Keys::ExportToZip) && BackupPath.GetExtension() != u"zip")
        {
            BackupPath += u".zip";
        }

        // To do: I don't like this, but I'm not sure if there's a much better way to clean this up.
        if (Config::GetByKey(Config::Keys::ExportToZip) || BackupPath.GetExtension() == u".zip")
        {
            zipFile ZipOut = zipOpen64("sdmc:/Temp.zip", APPEND_STATUS_CREATE);
            if (!ZipOut)
            {
                return;
            }
            JKSM::PushState(std::make_shared<ProgressTaskState>(this, CreateNewBackupZip, ZipOut, BackupPath, this));
        }
        else
        {
            JKSM::PushState(std::make_shared<ProgressTaskState>(this, CreateNewBackupFolder, BackupPath, this));
        }
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
    m_BackupMenu->Draw(Target);
}

void BackupMenuState::Refresh(void)
{
    m_DirectoryListing.Open(m_DirectoryPath);
    m_BackupMenu->Reset();

    // Loop and copy directory to menu after adding new
    m_BackupMenu->AddOption(UI::Strings::GetStringByName(UI::Strings::Names::FolderMenuNew, 0));
    for (uint32_t i = 0; i < m_DirectoryListing.GetEntryCount(); i++)
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_DirectoryListing.GetEntryAt(i).data(), UTF8Buffer, 0x80);
        m_BackupMenu->AddOption(UTF8Buffer);
    }
}

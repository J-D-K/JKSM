#include "AppStates/BackupMenuState.hpp"
#include "FS/FS.hpp"
#include "Fs/SaveMount.hpp"
#include "Input.hpp"
#include "SDL/SDL.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"

// We're going to mark this as a task even though it isn't so JKSM doesn't let users shift from it.
BackupMenuState::BackupMenuState(AppState *CreatingState, const Data::TitleData *Data, Data::SaveDataType SaveType)
    : AppState(AppState::StateTypes::Task), m_CreatingState(CreatingState), m_Data(Data),
      m_BackupMenu(std::make_unique<UI::Menu>(4, 20, 312, 12))
{
    m_DirectoryPath = FS::GetBasePath(SaveType) / Data->GetPathSafeTitle();
    if (!FsLib::DirectoryExists(m_DirectoryPath) && !FsLib::CreateDirectoriesRecursively(m_DirectoryPath))
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

    if (Input::ButtonPressed(KEY_B))
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

    // Loop and copy directory to menu after adding new
    m_BackupMenu->AddOption(UI::Strings::GetStringByName(UI::Strings::Names::FolderMenuNew, 0));
    for (uint32_t i = 0; i < m_DirectoryListing.GetEntryCount(); i++)
    {
        char UTF8Buffer[0x80] = {0};
        StringUtil::ToUTF8(m_DirectoryListing.GetEntryAt(i).data(), UTF8Buffer, 0x80);
        m_BackupMenu->AddOption(UTF8Buffer);
    }
}

#pragma once
#include "AppStates/AppState.hpp"
#include "Data/TitleData.hpp"
#include "FsLib.hpp"
#include "System/ProgressTask.hpp"
#include "UI/Menu.hpp"
#include <memory>
#include <mutex>

class BackupMenuState : public AppState
{
    public:
        BackupMenuState(AppState *CreatingState, const Data::TitleData *Data, Data::SaveDataType SaveType);
        ~BackupMenuState();

        void Update(void);
        void DrawTop(SDL_Surface *Target);
        void DrawBottom(SDL_Surface *Target);

        // Reloads listing and refreshes menu.
        void Refresh(void);

    private:
        // Pointer to state that created this one so we can draw the top screen.
        AppState *m_CreatingState = nullptr;
        // Keep the pointer just in case.
        const Data::TitleData *m_Data = nullptr;
        // Save data type we're working with.
        Data::SaveDataType m_SaveType;
        // Backup menu.
        std::unique_ptr<UI::Menu> m_BackupMenu = nullptr;
        // Directory Path
        FsLib::Path m_DirectoryPath;
        // Directory Listing.
        FsLib::Directory m_DirectoryListing;
        // X coordinate for centering bottom text header thingy.
        int m_TextX = 0;
        // Mutex to prevent corruption.
        std::mutex m_ListingMutex;
};

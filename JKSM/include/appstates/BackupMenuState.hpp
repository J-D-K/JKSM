#pragma once
#include "Data/TitleData.hpp"
#include "System/ProgressTask.hpp"
#include "UI/Menu.hpp"
#include "appstates/BaseState.hpp"
#include "fslib.hpp"

#include <memory>
#include <mutex>

class BackupMenuState final : public BaseState
{
    public:
        /// @brief Creates a new BackupMenu state instance.
        BackupMenuState(BaseState *creatingState, const Data::TitleData *data, Data::SaveDataType saveType);

        /// @brief Destructor. Going to be removed for scoped mounts.
        ~BackupMenuState();

        /// @brief Update override.
        void update() override;

        /// @brief Override to draw the top screen.
        void draw_top(SDL_Surface *target) override;

        /// @brief Override to draw the bottom screen.
        void draw_bottom(SDL_Surface *target) override;

        /// @brief Refresh override.
        void refresh();

    private:
        /// @brief Pointer to the spawning state so I can draw the top.
        BaseState *m_creatingState{};

        /// @brief Pointer to data just in case.
        const Data::TitleData *m_data{};

        /// @brief Save data type we're working with.
        Data::SaveDataType m_saveType{};

        /// @brief Menu.
        UI::Menu m_backupMenu;

        /// @brief Path pointing to the target directory.
        fslib::Path m_directoryPath{};

        /// @brief Listing of the target directory.
        fslib::Directory m_directoryListing{};

        /// @brief Centered coordinate for the header text.
        int m_textX{};

        /// @brief Mutex to guard against corruption.
        std::mutex m_listingMutex{};
};

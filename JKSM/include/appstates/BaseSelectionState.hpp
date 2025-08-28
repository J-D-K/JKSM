#pragma once
#include "Data/Data.hpp"
#include "appstates/BaseState.hpp"

#include <vector>

class BaseSelectionState : public BaseState
{
    public:
        BaseSelectionState(Data::SaveDataType saveType);

        ~BaseSelectionState() {};

        /// @brief Virtual passthrough.
        void update() = 0;

        /// @brief Draw top passthrough.
        void draw_top(SDL_Surface *target) = 0;

        /// @brief Draw bottom passthrough.
        void draw_bottom(SDL_Surface *target) = 0;

        /// @brief All selection state types need this.
        virtual void refresh() = 0;

        /// @brief Draws the title's information on the bottom screen.
        void draw_title_info(SDL_Surface *target, const Data::TitleData *data);

        /// @brief Creates a backup menu using the data passed.
        void create_backup_state(const Data::TitleData *data);

        /// @brief Creates an option state using the data passed.
        void create_option_state(const Data::TitleData *data);

    protected:
        // Save data type
        Data::SaveDataType m_saveType;

        /// @brief Mounts the data passed using data according to m_saveType;
        bool mount_save_data(const Data::TitleData *data);
};

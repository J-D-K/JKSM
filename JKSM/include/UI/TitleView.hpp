#pragma once
#include "Data/Data.hpp"
#include "UI/Element.hpp"
#include "UI/TitleTile.hpp"
#include <vector>

namespace UI
{
    class TitleView : public UI::Element
    {
        public:
            TitleView(Data::SaveDataType SaveType);
            ~TitleView() {};

            void Update(void);
            void Draw(SDL_Surface *Target);

            // Forces a complete refresh of view
            void Refresh(void);

            // Allows you to set the selected tile.
            void SetSelected(int Selected);
            // Returns index of selected tile.
            int GetSelected(void) const;

        private:
            // Save type of view
            Data::SaveDataType m_SaveType;
            // Currently selected title.
            int m_Selected = 0;
            // X and Y coordinates to begin drawing at.
            int m_X = 14, m_Y = 24;
            // X and Y coordinates of selection bounding box.
            int m_SelectionX = 0, m_SelectionY = 0;
            // Vector of pointers to title data. I don't want to make copies.
            std::vector<Data::TitleData *> m_TitleData;
            // Tile vector.
            std::vector<UI::TitleTile> m_TitleTiles;
    };
} // namespace UI

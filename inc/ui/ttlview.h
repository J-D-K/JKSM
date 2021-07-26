#pragma once

#include <vector>
#include "data.h"
#include "type.h"

namespace ui
{
    class titleTile
    {
        public:
            titleTile(bool _fav, C2D_Image *_icon)
            {
                icon = _icon;
                fav  = _fav;
            }

            void draw(int x, int y, bool sel, uint8_t clrShft);

        private:
            C2D_Image *icon;
            bool fav = false;
    };

    class titleview
    {
        public:
            titleview(std::vector<data::titleData>& _t, funcPtr _cb, void *_args);
            ~titleview();

            void update();
            void refesh(std::vector<data::titleData>& _t);
            void draw();
            size_t debGetSize(){ return tiles.size(); }

            void setSelected(int _sel) { selected = _sel; }
            int getSelected() { return selected; }

        private:
            int selected = 0, x = 14, y = 32;
            int selRectX = 0, selRectY = 0;
            bool clrAdd = true;
            uint8_t clrShft = 0;
            std::vector<titleTile> tiles;
            funcPtr callback;
            void *cbArgs;
    };
}

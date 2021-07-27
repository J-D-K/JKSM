#include <cmath>
#include <citro2d.h>

#include "gfx.h"
#include "ui.h"
#include "ui/ttlview.h"

void ui::titleTile::draw(int x, int y, bool sel, uint8_t clrShft)
{
    if(sel)
    {
        uint32_t bbClr = 0xFF << 24 | (uint8_t)(0xC5 + (clrShft / 2)) << 16 | (uint8_t)(0x88 + clrShft) << 8 | 0x00;
        gfx::drawBoundingBox(x - 3, y - 3, 54, 54, 0.6f, bbClr, false);
        C2D_DrawImageAt(*icon, (float)x, (float)y, 0.6f);
    }
    else
        C2D_DrawImageAt(*icon, (float)x, (float)y, 0.5f);

    if(fav)
        gfx::drawText("♥", x + 2, y + 2, 0.7f, 0.4f, 0xFF4444FF);
}

ui::titleview::titleview(std::vector<data::titleData>& _t, funcPtr _cb, void *args)
{
    callback = _cb;
    cbArgs = args;
    for(data::titleData& d : _t)
        tiles.emplace_back(d.getFav(), d.getIcon());
}

ui::titleview::~titleview()
{
    tiles.clear();
}

void ui::titleview::update()
{
    int tileTotal = tiles.size() - 1;

    uint32_t down = ui::padKeysDown();

    switch(down)
    {
        case KEY_DUP:
            if((selected -= 7) < 0)
                selected = 0;
            break;

        case KEY_DDOWN:
            if((selected += 7) > tileTotal)
                selected = tileTotal;
            break;

        case KEY_DLEFT:
            if(--selected < 0)
                selected = 0;
            break;

        case KEY_DRIGHT:
            if(++selected > tileTotal)
                selected = tileTotal;
            break;

        case KEY_L:
            if((selected -= 21) < 0)
                selected = 0;
            break;

        case KEY_R:
            if((selected += 21) > tileTotal)
                selected = tileTotal;
            break;
    }

    if(callback)
        (*(callback))(cbArgs);
}

void ui::titleview::refesh(std::vector<data::titleData>& _t)
{
    tiles.clear();

    for(data::titleData& d : _t)
        tiles.emplace_back(d.getFav(), d.getIcon());
}

void ui::titleview::draw()
{
    if(tiles.size() <= 0)
        return;

    if(clrAdd && (clrShft += 6) >= 0x72)
        clrAdd = false;
    else if(!clrAdd && (clrShft -= 3) <= 0x00)
        clrAdd = true;

    //Target Y now, temp Y later
    int tY = 176;
    if(selRectY > tY)
    {
        float add = ((float)tY - (float)selRectY) / 3.0f;
        y += ceil(add);
    }
    else if(selRectY < 21)
    {
        float add = (21.0f - (float)selRectY) / 3.0f;
        y += ceil(add);
    }

    int tileTotal = tiles.size();
    for(int tY = y, i = 0; i < tileTotal; tY += 56)
    {
        int endRow = i + 7;
        for(int tX = x; i < endRow; tX += 54, i++)
        {
            if(i >= tileTotal)
                break;

            if(i == selected)
            {
                selRectX = tX - 4;
                selRectY = tY - 4;
                tiles[i].draw(tX, tY, true, clrShft);
            }
            else
                tiles[i].draw(tX, tY, false, 0);
        }
    }
}

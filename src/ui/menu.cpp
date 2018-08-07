#include <string>

#include "ui/menu.h"
#include "gfx.h"

namespace ui
{
    void menu::addOpt(const std::string& add, int maxWidth)
    {
        if((int)gfx::getTextWidth(add) < maxWidth || maxWidth == 0)
            opt.push_back(add);
        else
        {
            std::string tmp;
            for(unsigned i = 0; i < add.length(); i++)
            {
                tmp += add[i];
                if((int)gfx::getTextWidth(tmp) >= maxWidth)
                {
                    tmp.replace(i - 2, 3, "...");
                    opt.push_back(tmp);
                    break;
                }
            }
        }
    }

    void menu::reset()
    {
        opt.clear();

        selected = 0;
        start = 0;
    }

    void menu::setSelected(const int& newSel)
    {
        if(newSel < start || newSel > start + 16)
        {
            int size = opt.size() - 1;
            if(newSel + 16 > size)
                start = size - 16;
            else
                start = newSel;

            selected = newSel;
        }
        else
            selected = newSel;
    }

    void menu::handleInput(const uint32_t& down, const uint32_t& held)
    {
        if( (held & KEY_UP) || (held & KEY_DOWN))
            fc++;
        else
            fc = 0;
        if(fc > 10)
            fc = 0;

        int size = opt.size() - 1;
        if((down & KEY_UP) || ((held & KEY_UP) && fc == 10))
        {
            selected--;
            if(selected < 0)
                selected = size;

            if(size < 17)
                start = 0;
            else if(start > selected)
                start--;
            else if(selected == size && size > 16)
                start = size - 16;
        }
        else if((down & KEY_DOWN) || ((held & KEY_DOWN) && fc == 10))
        {
            selected++;
            if(selected > size)
                selected = 0;

            if((selected > (start + 16)) && ((start + 16) < size))
                start++;
            if(selected == 0)
                start = 0;
        }
        else if(down & KEY_RIGHT)
        {
            selected += 8;
            if(selected > size)
                selected = size;
            if((selected - 16) > start)
                start = selected - 16;
        }
        else if(down & KEY_LEFT)
        {
            selected -= 8;
            if(selected < 0)
                selected = 0;
            if(selected < start)
                start = selected;
        }
    }

    void menu::draw(const int& x, const int& y, const uint32_t& baseClr, const uint32_t& rectWidth)
    {
        if(clrAdd)
        {
            clrSh += 4;
            if(clrSh > 63)
                clrAdd = false;
        }
        else
        {
            clrSh--;
            if(clrSh == 0)
                clrAdd = true;
        }

        int length = 0;
        if((opt.size() - 1) < 17)
            length = opt.size();
        else
            length = start + 17;

        uint32_t rectClr = 0xFF << 24 | ((0xBB + clrSh) & 0xFF) << 16 | ((0x88 + clrSh) << 8) | 0x00;

        for(int i = start; i < length; i++)
        {
            if(i == selected)
                C2D_DrawRectSolid(x, (y + 2) + ((i - start) * 12), 0.5f, rectWidth, 12, rectClr);

            gfx::drawText(opt[i], x, y + ((i - start) * 12), baseClr);
        }
    }
}

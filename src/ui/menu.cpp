#include <string>

#include "ui/menu.h"
#include "gfx.h"

namespace ui
{
    void menu::addOpt(const std::string& add, int maxWidth)
    {
        if((int)gfx::getTextWidth(add) < maxWidth - 32 || maxWidth == 0)
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

        if(multi)
            multiSel.push_back(false);
    }

    void menu::multiSet(const bool& s)
    {
        multi = s;
    }

    void menu::reset()
    {
        opt.clear();
        multiSel.clear();
        multi = false;
    }

    void menu::adjust()
    {
        if(selected > (int)opt.size() - 1)
            selected = opt.size() - 1;

        if(opt.size() < 12)
            start = 0;
        else if(opt.size() > 11 && start + 11 > (int)opt.size() - 1)
            start--;
    }

    void menu::setSelected(const int& newSel)
    {
        if(newSel < start || newSel > start + 11)
        {
            int size = opt.size() - 1;
            if(newSel + 11 > size)
                start = size - 11;
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

            if(size < 12)
                start = 0;
            else if(start > selected)
                start--;
            else if(selected == size && size > 11)
                start = size - 11;
        }
        else if((down & KEY_DOWN) || ((held & KEY_DOWN) && fc == 10))
        {
            selected++;
            if(selected > size)
                selected = 0;

            if((selected > (start + 11)) && ((start + 11) < size))
                start++;
            if(selected == 0)
                start = 0;
        }
        else if(down & KEY_RIGHT)
        {
            selected += 7;
            if(selected > size)
                selected = size;
            if((selected - 11) > start)
                start = selected - 11;
        }
        else if(down & KEY_LEFT)
        {
            selected -= 7;
            if(selected < 0)
                selected = 0;
            if(selected < start)
                start = selected;
        }
        else if(down & KEY_L && multi)
        {
            if(multiSel[selected])
                multiSel[selected] = false;
            else
                multiSel[selected] = true;
        }
    }

    void menu::draw(const int& x, const int& y, const uint32_t& baseClr, const uint32_t& rectWidth, bool lightBack)
    {
        if(clrAdd)
        {
            clrSh += 6;
            if(clrSh >= 0xFA)
                clrAdd = false;
        }
        else
        {
            clrSh -= 3;
            if(clrSh <= 0x88)
                clrAdd = true;
        }

        int length = 0;
        if((opt.size() - 1) < 12)
            length = opt.size();
        else
            length = start + 12;

        uint32_t rectClr = 0xFF << 24 | 0xC5 << 16 | clrSh << 8 | 0x00;

        for(int i = start; i < length; i++)
        {
            if(i == selected)
            {
                gfx::drawBoundingBox(x, (y - 2) + ((i - start) * 18), rectWidth, 18, rectClr, lightBack);
                C2D_DrawRectSolid(x + 4, y + 1 + ((i - start) * 18), 0.5f, 2, 12, 0xFFC5FF00);
            }
            else if(multi && multiSel[i])
            {
                gfx::drawBoundingBox(x, (y - 2) + ((i - start) * 18), rectWidth, 18, 0xFFC58800, lightBack);
                C2D_DrawRectSolid(x + 4, y + 1 + ((i - start) * 18), 0.5f, 2, 12, 0xFFC5FF00);
            }

            gfx::drawText(opt[i], x + 8, (y - 1) + ((i - start) * 18), baseClr);
        }
    }
}

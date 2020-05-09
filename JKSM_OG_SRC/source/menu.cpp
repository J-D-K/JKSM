#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>
#include <string.h>

#include "global.h"
#include "menu.h"

#define FONT_SIZE 12

menuItem::menuItem(const std::u32string s, bool center, int _x)
{
    selected = false;
    text.assign(s);
    if(center)
        autoCenter();
    else
        x = _x;
}

void menuItem::autoCenter()
{
    x = (400 / 2) - (sftd_get_wtext_width(font, 12, (wchar_t *)text.data()) / 2);
    if(x < 0)
        x = 0;
}

void menuItem::draw(int y, unsigned color)
{
    sftd_draw_wtext(font, x, y, color, 12, (wchar_t *)text.data());
}

menu::menu(unsigned sx, unsigned sy, bool sMulti, bool _center)
{
    x = sx;
    y = sy;
    multi = sMulti;
    center = _center;
    selected = 0;
    start = 0;
    fc = 0;
}

menu::~menu()
{
    opts.clear();
}

void menu::addItem(const char *a)
{
    char32_t *tmp = new char32_t[128];
    memset(tmp, 0, sizeof(char32_t) * 128);
    utf8_to_utf32((uint32_t *)tmp, (uint8_t *)a, 128);

    std::u32string t;
    t.assign(tmp);

    menuItem add(t, center, x);

    opts.push_back(add);

    delete[] tmp;
}

void menu::addItem(const std::u16string a)
{
    char32_t *tmp = new char32_t[128];
    memset(tmp, 0, sizeof(char32_t) * 128);
    utf16_to_utf32((uint32_t *)tmp, (uint16_t *)a.data(), 128);

    std::u32string t;
    t.assign(tmp);

    menuItem add(t, center, x);

    opts.push_back(add);

    delete[] tmp;
}

void menu::updateItem(int i, const char *a)
{
    char32_t tmp[128];
    memset(tmp, 0, 128);

    utf8_to_utf32((uint32_t *)tmp, (uint8_t *)a, 128);

    menuItem update(tmp, center, x);

    opts[i] = update;
}

void menu::draw()
{
    int i, length;

    //Set length for printing
    if((opts.size() - 1) < 15)
        length = opts.size();
    else
        //Scrolling needs start set
        length = start + 15;

    for(i = start; i < length; i++)
    {
        if(i == selected)
        {
            opts[i].draw(y + ((i - start) * 14), RGBA8(selColor[0], selColor[1], selColor[2], 255));
        }
        else if(opts[i].selected)
            opts[i].draw(y + ((i - start) * 14), RGBA8(200, 0, 0, 255));
        else
            opts[i].draw(y + ((i - start) * 14), RGBA8(unSelColor[0], unSelColor[1], unSelColor[2], 255));
    }

}

void menu::handleInput(u32 key, u32 held)
{
    //Honestly. I don't even know anymore.
    //I stopped touching this. It works, but it's a mess.

    //keep frame count for scrolling
    if( (held & KEY_UP) || (held & KEY_DOWN))
        fc++;
    else
        fc = 0;
    if(fc > 10)
        fc = 0;

    int size = opts.size() - 1;
    if((key & KEY_UP) || ((held & KEY_UP) && fc == 10))
    {
        selected--;
        if(selected < 0)
            selected = size;

        if((start > selected)  && (start > 0))
            start--;
        if(size < 15)
            start = 0;
        else if(selected == size && size > 15)
            start = size - 14;
    }
    else if((key & KEY_DOWN) || ((held & KEY_DOWN) && fc == 10))
    {
        selected++;
        if(selected > size)
            selected = 0;

        if((selected > (start + 14)) && ((start + 14) < size))
            start++;
        if(selected == 0)
            start = 0;
    }
    else if(key & KEY_RIGHT)
    {
        selected += 7;
        if(selected > size)
            selected = size;
        if((selected - 14) > start)
            start = selected - 14;
    }
    else if(key & KEY_LEFT)
    {
        selected -= 7;
        if(selected < 0)
            selected = 0;
        if(selected < start)
            start = selected;
    }
    else if((key & KEY_L) && multi)
    {
        if(opts[selected].selected)
            opts[selected].selected = false;
        else
            opts[selected].selected = true;
    }
    else if((key & KEY_R) && multi)
    {
        if(opts[0].selected && opts[1].selected)
        {
            for(unsigned i = 0; i < opts.size(); i++)
                opts[i].selected = false;
        }
        else
        {
            for(unsigned i = 0; i < opts.size(); i++)
                opts[i].selected = true;
        }
    }

}

void menu::reset()
{
    selected = 0;
    start = 0;
    opts.clear();
}

void menu::centerOpts()
{
    for(unsigned i = 0; i < opts.size(); i++)
        opts[i].autoCenter();
}

void menu::autoVert()
{

    unsigned usedVert;
    if(getSize() > 15)
        usedVert = 15 * 14;
    else
        usedVert = getSize() * 14;
    y = 16 + (224 / 2) - (usedVert / 2);
}

int menu::getSelected()
{
    return selected;
}

void menu::setSelected(int sel)
{
    selected = sel;
}

unsigned menu::getSize()
{
    return opts.size();
}

unsigned menu::getSelectCount()
{
    unsigned ret = 0;
    for(unsigned i = 0; i < opts.size(); i++)
    {
        if(opts[i].selected)
            ret++;
    }

    return ret;
}

bool menu::optSelected(int i)
{
    return opts[i].selected;
}

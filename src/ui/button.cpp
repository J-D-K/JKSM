#include <3ds.h>

#include "button.h"
#include "gfx.h"
#include "util.h"

ui::button::button(const std::string &_txt, unsigned _x, unsigned _y, unsigned _w, unsigned _h)
{
    x = _x;
    y = _y;
    w = _w;
    h = _h;
    text = _txt;

    unsigned tw = gfx::getTextWidth(text);
    unsigned th = 14;

    tx = x + (w / 2) - (tw / 2);
    ty = y + (h / 2) - (th / 2);
}

void ui::button::update()
{
    prev = cur;
    cur = ui::touchPosition();

    // If button was first thing pressed
    if (isOver() && prev.px == 0 && prev.py == 0)
    {
        first = true;
        pressed = true;
        retEvent = BUTTON_PRESSED;
    }
    else if (retEvent == BUTTON_PRESSED && !util::touchPressed(p) && wasOver())
    {
        first = false;
        pressed = false;
        retEvent = BUTTON_RELEASED;
    }
    else if (retEvent != BUTTON_NOTHING && !util::touchPressed(p))
    {
        first = false;
        pressed = false;
        retEvent = BUTTON_NOTHING;
    }
}

bool ui::button::isOver()
{
    return (cur.px > x && cur.px < x + w && cur.py > y && cur.py < y + h);
}

bool ui::button::wasOver()
{
    return (prev.px > x && prev.px < x + w && prev.py > y && prev.py < y + h);
}

void ui::button::draw()
{
    if (pressed)
        C2D_DrawRectSolid(x, y, GFX_DEPTH_DEFAULT, w, h, 0xFFBBBBBB);
    else
        C2D_DrawRectSolid(x, y, GFX_DEPTH_DEFAULT, w, h, 0xFFDBDBDB);

    gfx::drawText(text, tx, ty, GFX_DEPTH_DEFAULT, 0.5f, 0xFF000000);
}

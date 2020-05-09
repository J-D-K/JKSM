#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>
#include <stdio.h>
#include <string.h>

#include "ui.h"
#include "global.h"
#include "img.h"
#include "util.h"
#include "date.h"

bool confirm(const char *t)
{
    bool ret = false;

    button yes("Yes (A)", 40, 176, 96, 32);
    button no("No (B)", 180, 176, 96, 32);
    textbox back(24, 24, 280, 200, t, "Confirm");
    while(true)
    {
        hidScanInput();

        u32 down = hidKeysDown();

        touchPosition p;
        hidTouchRead(&p);

        if(yes.released(p) || (down & KEY_A))
        {
            ret = true;
            break;
        }
        else if(no.released(p) || (down & KEY_B))
        {
            ret = false;
            break;
        }

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        back.draw();
        yes.draw();
        no.draw();
        sf2d_end_frame();

        sf2d_swapbuffers();
    }

    return ret;
}

void showMessage(const char *t, const char *head)
{
    button ok("OK (A)", 116, 176, 96, 32);
    textbox back(24, 24, 280, 200, t, head);

    while(true)
    {
        hidScanInput();

        touchPosition p;
        hidTouchRead(&p);

        u32 down = hidKeysDown();

        if(ok.released(p) || down & KEY_A)
            break;

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        back.draw();
        ok.draw();
        sf2d_end_frame();

        sf2d_swapbuffers();
    }
}

void showError(const char *t, unsigned error)
{
    char full[256];
    sprintf(full, "%s : %08X", t, error);
    button ok("OK (A)", 116, 176, 96, 32);
    textbox back(24, 24, 280, 200, full, "Error:");

    while(true)
    {
        hidScanInput();

        touchPosition p;
        hidTouchRead(&p);

        u32 down = hidKeysDown();

        if(ok.released(p) || down & KEY_A)
            break;

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        back.draw();
        ok.draw();
        sf2d_end_frame();

        sf2d_swapbuffers();
    }
}

static sf2d_texture *progFull, *tbox, *bar;

void textboxInit()
{
    tbox = sf2d_create_texture(48, 56, TEXFMT_RGBA8, SF2D_PLACE_RAM);

    memcpy(tbox->tex.data, textboxData, 0x4000);
}

void textboxExit()
{
    sf2d_free_texture(tbox);
}

textbox::textbox(unsigned x, unsigned y, unsigned width, unsigned height, const char *text, const char *_head)
{
    Text = text;
    evenString(&Text);
    head = _head;

    X = x;
    Y = y;
    Width = width;
    Height = height;

    headX =  X + ((width / 2) - (sftd_get_text_width(font, 12, head.c_str()) / 2));

    if(Width > 32)
        xScale = (float)((Width - 32) / 16);
    if(Height > 32)
        yScale = (float)((Height - 32) / 16);
}

void textbox::draw()
{
    //Top
    sf2d_draw_texture_part(tbox, X, Y, 0, 0, 16, 24);
    sf2d_draw_texture_part_scale(tbox, X + 16, Y, 16, 0, 16, 24, xScale, 1);
    sf2d_draw_texture_part(tbox, (X + 16) + (16 * xScale), Y, 32, 0, 16, 24);

    //Mid
    sf2d_draw_texture_part_scale(tbox, X, Y + 24, 0, 24, 16, 16, 1, yScale);
    sf2d_draw_texture_part_scale(tbox, X + 16, Y + 24, 16, 24, 16, 16, xScale, yScale);
    sf2d_draw_texture_part_scale(tbox, (X + 16) + (16 * xScale), Y + 24, 32, 24, 16, 16, 1, yScale);

    //Bottom
    sf2d_draw_texture_part(tbox, X, (Y + 16) + (16 * yScale), 0, 40, 16, 16);
    sf2d_draw_texture_part_scale(tbox, X + 16, (Y + 16) + (16 * yScale), 16, 40, 16, 16, xScale, 1);
    sf2d_draw_texture_part(tbox, (X + 16) + (16 * xScale), (Y + 16) + (16 * yScale), 32, 40, 16, 16);

    //Text
    sftd_draw_text(font, headX, Y + 4, RGBA8(0, 0, 0, 255), 12, head.c_str());
    sftd_draw_text_wrap(font, X + 8, Y + 26, RGBA8(0, 0, 0, 255), 12, (X + Width) - 8, Text.c_str());
}


void progressBarInit()
{
    progFull = sf2d_create_texture(2, 16, TEXFMT_RGBA8, SF2D_PLACE_RAM);

    memcpy(progFull->tex.data, progBarData, 0x1000);
}

void progressBarExit()
{
    sf2d_free_texture(progFull);
}

progressBar::progressBar(float _max, const char *t, const char *head)
{
    back = new textbox(24, 24, 280, 200, t, head);
    max = _max;
}

progressBar::~progressBar()
{
    delete back;
}

void progressBar::draw(float cur)
{
    float prog = (float)(cur / max) * 100;
    float xScale = (float)(prog * 128) / 100;

    back->draw();
    sf2d_draw_rectangle(back->X + 8, back->Y + 160, 256, 16, RGBA8(0, 0, 0, 255));
    sf2d_draw_texture_scale(progFull, back->X + 8, back->Y + 160, xScale, 1);
}

button::button(const char * sText, int sX, int sY, int sWidth, int sHeight)
{
    X = sX;
    Y = sY;
    width = sWidth;
    height = sHeight;
    text = sText;

    textX =  X + 8;
    textY = Y + 8;

    Pressed = false;
}

void button::draw()
{
    sf2d_draw_rectangle(X - 1, Y - 1, width + 2, height + 2, RGBA8(64, 64, 64, 255));
    if(Pressed)
        sf2d_draw_rectangle(X, Y, width, height, RGBA8(200, 200, 200, 255));
    else
        sf2d_draw_rectangle(X, Y, width, height, RGBA8(244, 244, 244, 255));

    sftd_draw_text(font, textX, textY, RGBA8(0, 0, 0, 255), 12, text.c_str());
}

//This checks whether the stylus is inside the button
bool button::isOver(touchPosition p)
{
    return (p.px > X && p.px < X + width) && (p.py > Y && p.py < Y + height);
}

//Checks if the stylus was removed.
bool button::released(touchPosition p)
{
    Prev = p;
    if(isOver(p))
    {
        Pressed = true;
    }
    else
    {
        //This should now only return true if
        //the stylus is lifted directly off.
        //should return false now if moved away to different part
        if(Pressed && !touchPressed(p))
        {
            Pressed = false;
            return true;
        }
        else
        {
            Pressed = false;
        }
    }
    return false;
}

void topBarInit()
{
    bar = sf2d_create_texture(16, 16, TEXFMT_RGBA8, SF2D_PLACE_RAM);

    memcpy(bar->tex.data, topbarData, 0x1000);
}

void topBarExit()
{
    sf2d_free_texture(bar);
}

void drawTopBar(const std::u32string nfo)
{
    //the bar
    sf2d_draw_texture_scale(bar, 0, 0, 25, 1);

    //nfo is wide text, UTF32
    sftd_draw_wtext(font, 0, 0, RGBA8(0, 0, 0, 255), 12, (wchar_t *)nfo.data());

    //time
    sftd_draw_text(font, 360, 0, RGBA8(0, 0, 0, 255), 12, RetTime().c_str());
}

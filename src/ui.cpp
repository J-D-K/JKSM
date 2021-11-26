#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>

#include <stdio.h>

#include "data.h"
#include "gfx.h"
#include "ui.h"
#include "util.h"
#include "fs.h"
#include "sys.h"
#include "smdh.h"

#include "ui/ttlview.h"
#include "ui/ttl.h"

//8
const std::string ui::loadGlyphArray[] =
{
    "\ue020", "\ue021", "\ue022", "\ue023",
    "\ue024", "\ue025", "\ue026", "\ue027"
};

static ui::menu dolderMenu;

int ui::state = DAT, ui::prev = DAT;

const std::string TITLE_TEXT = "JK's Save Manager - 11.25.2021 ";

uint32_t ui::down = 0, ui::held = 0;

void ui::init()
{
    ui::ttlInit();
    ui::extInit();
    ui::sysInit();
    ui::bossViewInit();
    ui::shrdInit();
    ui::setInit();
}

void ui::exit()
{
    ui::ttlExit();
    ui::extExit();
    ui::sysExit();
    ui::bossViewExit();
    ui::shrdExit();
    ui::setExit();
}

void ui::drawUIBar(const std::string& txt, int screen, bool center)
{
    unsigned topX = 4, botX = 4;
    switch(screen)
    {
        case ui::SCREEN_TOP://top
            topX = 200 - (gfx::getTextWidth(txt) / 2);
            C2D_DrawRectSolid(0, 0, 0.8f, 400, 16, 0xFF505050);
            C2D_DrawRectSolid(0, 16, 0.8f, 400, 1, 0x881D1D1D);
            gfx::drawText(txt, topX, 0, 0.8f, 0.5f, 0xFFFFFFFF);
            break;

        case ui::SCREEN_BOT://bottom
            botX = 160 - (gfx::getTextWidth(txt) / 2);
            C2D_DrawRectSolid(0, 224, 0.8f, 400, 16, 0xFF505050);
            C2D_DrawRectSolid(0, 223, 0.8f, 400, 1, 0x881D1D1D);
            gfx::drawText(txt, botX, 224, 0.8f, 0.5f, 0xFFFFFFFF);
            break;
    }
}

void drawUI()
{
    gfx::frameBegin();
    gfx::frameStartTop();
    switch(ui::state)
    {
        case DAT:
            data::datDrawTop();
            break;

        case USR:
            ui::ttlDrawTop();
            break;

        case EXT:
            ui::extDrawTop();
            break;

        case SYS:
            ui::sysDrawTop();
            break;

        case BOS:
            ui::bossViewDrawTop();
            break;

        case SHR:
            ui::shrdDrawTop();
            break;

        case SET:
            ui::setDrawTop();
            break;

        default:
            break;
    }
    gfx::frameStartBot();
    switch(ui::state)
    {
        case DAT:
            data::datDrawBot();
            break;

        case USR:
            ui::ttlDrawBot();
            break;

        case EXT:
            ui::extDrawBot();
            break;

        case SYS:
            ui::sysDrawBot();
            break;

        case BOS:
            ui::bossViewDrawBot();
            break;

        case SHR:
            ui::shrdDrawBot();
            break;

        case SET:
            ui::setDrawBottom();
            break;

        default:
            break;
    }
    gfx::frameEnd();
}

bool ui::runApp()
{
    ui::updateInput();

    data::cartCheck();

    if(ui::padKeysDown() & KEY_START)
        return false;

    switch(state)
    {
        case USR:
            ui::ttlUpdate();
            break;

        case EXT:
            ui::extUpdate();
            break;

        case SYS:
            ui::sysUpdate();
            break;

        case BOS:
            ui::bossViewUpdate();
            break;

        case SHR:
            ui::shrdUpdate();
            break;

        case SET:
            ui::setUpdate();
            break;
        }

    drawUI();

    return true;
}

void ui::showMessage(const char *fmt, ...)
{
    char tmp[512];
    va_list args;
    va_start(args, fmt);
    vsprintf(tmp, fmt, args);

    ui:: button ok("OK \ue000", 96, 192, 128, 32);
    while(1)
    {
        hidScanInput();

        uint32_t down = hidKeysDown();
        touchPosition p;
        hidTouchRead(&p);

        ok.update(p);

        if(down & KEY_A || ok.getEvent() == BUTTON_RELEASED)
            break;

        gfx::frameBegin();
        gfx::frameStartBot();
        C2D_DrawRectSolid(8, 8, 0.5f, 304, 224, 0xFFE7E7E7);
        ok.draw();
        gfx::drawTextWrap(tmp, 16, 16, 300, GFX_DEPTH_DEFAULT, 0.5f, 0xFF000000);
        gfx::frameEnd();
    }
}

ui::progressBar::progressBar(const uint32_t& _max)
{
    max = (float)_max;
}

void ui::progressBar::update(const uint32_t& _prog)
{
    prog = (float)_prog;

    float percent = (float)(prog / max) * 100;
    width  = (float)(percent * 288) / 100;
}

void ui::progressBar::draw(const std::string& text)
{
    C2D_DrawRectSolid(8, 8, 0.5f, 304, 224, 0xFFAAAAAA);
    gfx::drawTextWrap(text, 16, 16, 0.5f, 0.5f, 240, 0xFF000000);
    C2D_DrawRectSolid(16, 200, 1.0f, 288, 16, 0xFF000000);
    C2D_DrawRectSolid(16, 200, 1.0f, width, 16, 0xFF00FF00);
}

bool ui::confirm(const std::string& mess)
{
    button yes("Yes \ue000", 16, 192, 128, 32);
    button no("No \ue001", 176, 192, 128, 32);

    while(true)
    {
        hidScanInput();

        uint32_t down = hidKeysDown();
        touchPosition p;
        hidTouchRead(&p);

        //Oops
        yes.update(p);
        no.update(p);

        if(down & KEY_A || yes.getEvent() == BUTTON_RELEASED)
            return true;
        else if(down & KEY_B || no.getEvent() == BUTTON_RELEASED)
            return false;

        gfx::frameBegin();
        gfx::frameStartBot();
        C2D_DrawRectSolid(8, 8, 0.5f, 304, 224, 0xFFF4F4F4);
        gfx::drawTextWrap(mess, 16, 16, 300, GFX_DEPTH_DEFAULT, 0.5f, 0xFF000000);
        yes.draw();
        no.draw();
        gfx::frameEnd();
    }
    return false;
}

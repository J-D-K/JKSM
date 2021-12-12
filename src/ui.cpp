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

int ui::state = DAT, ui::prev = DAT;

static ui::threadProcMngr *thrdMgr;
static ui::button *ok, *yes, *no;

const std::string TITLE_TEXT = "JK's Save Manager - 12.11.2021 ";

uint32_t ui::down = 0, ui::held = 0;
touchPosition ui::p;

void ui::init()
{
    thrdMgr = new ui::threadProcMngr;
    ok = new ui::button("OK \ue000", 96, 184, 128, 32);
    yes = new ui::button("Yes \ue000", 32, 184, 120, 32);
    no  = new ui::button("No \ue001", 168, 184, 120, 32);
}

void ui::exit()
{
    ui::ttlExit();
    ui::extExit();
    ui::sysExit();
    ui::bossViewExit();
    ui::shrdExit();
    ui::setExit();
    delete thrdMgr;
    delete ok;
    delete yes;
    delete no;
}

void ui::drawUIBar(const std::string& txt, int screen, bool center)
{
    unsigned topX = 4, botX = 4;
    switch(screen)
    {
        case ui::SCREEN_TOP://top
            topX = 200 - (gfx::getTextWidth(txt) / 2);
            C2D_DrawRectSolid(0, 0, GFX_DEPTH_DEFAULT, 400, 16, 0xFF505050);
            C2D_DrawRectSolid(0, 16, GFX_DEPTH_DEFAULT, 400, 1, 0x881D1D1D);
            gfx::drawText(txt, topX, 0, GFX_DEPTH_DEFAULT, 0.5f, 0xFFFFFFFF);
            break;

        case ui::SCREEN_BOT://bottom
            botX = 160 - (gfx::getTextWidth(txt) / 2);
            C2D_DrawRectSolid(0, 224, GFX_DEPTH_DEFAULT, 400, 16, 0xFF505050);
            C2D_DrawRectSolid(0, 223, GFX_DEPTH_DEFAULT, 400, 1, 0x881D1D1D);
            gfx::drawText(txt, botX, 224, GFX_DEPTH_DEFAULT, 0.5f, 0xFFFFFFFF);
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
    if(!thrdMgr->empty())
        thrdMgr->drawTop();

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
    if(!thrdMgr->empty())
        thrdMgr->drawBot();

    gfx::frameEnd();
}

bool ui::runApp()
{
    ui::updateInput();

    thrdMgr->update();
    if(thrdMgr->empty())
    {
        
        if(ui::padKeysDown() & KEY_START)
        return false;

        data::cartCheck();

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
    }

    drawUI();

    return true;
}

void ui::showMessage(const char *fmt, ...)
{

}

void ui::newThread(ThreadFunc _thrdFunc, void *_args, funcPtr _drawFunc)
{
    thrdMgr->newThread(_thrdFunc, _args, _drawFunc);
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
    C2D_DrawRectSolid(8, 8, GFX_DEPTH_DEFAULT, 304, 224, 0xFFF4F4F4);
    gfx::drawTextWrap(text, 16, 16, GFX_DEPTH_DEFAULT, 0.5f, 240, 0xFF000000);
    C2D_DrawRectSolid(16, 200, GFX_DEPTH_DEFAULT, 288, 16, 0xFF000000);
    C2D_DrawRectSolid(16, 200, GFX_DEPTH_DEFAULT, width, 16, 0xFF00FF00);
}

void confirm_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    ui::confArgs *in = (ui::confArgs *)t->argPtr;
    while(true)
    {
        uint32_t down = ui::padKeysDown();
        yes->update();
        no->update();

        if((down & KEY_A || yes->getEvent() == BUTTON_RELEASED))
        {
            if(in->onConfirm)
                ui::newThread(in->onConfirm, in->args, NULL);
            break;
        }
        else if((down & KEY_B || no->getEvent() == BUTTON_RELEASED))
        {
            if(in->onCancel)
                (*in->onCancel)(in->args);
            break;
        }
        svcSleepThread(1e+9 / 60);
    }
    t->lock();
    delete in;
    t->argPtr = NULL;
    t->unlock();
    t->finished = true;
}

static void confirmDrawFunc(void *a)
{
    threadInfo *t = (threadInfo *)a;
    if(t->argPtr && t->running)
    {
        ui::confArgs *in = (ui::confArgs *)t->argPtr;
        C2D_DrawRectSolid(24, 24, GFX_DEPTH_DEFAULT, 272, 200, 0xFFE7E7E7);
        gfx::drawTextWrap(in->q, 32, 32, GFX_DEPTH_DEFAULT, 0.5f, 240, 0xFF000000);
        yes->draw();
        no->draw();
    }
}

void ui::confirm(const std::string& mess, funcPtr _onConfirm, funcPtr _onCancel, void *args)
{
    confArgs *send = new confArgs;
    send->q = mess;
    send->onConfirm = _onConfirm;
    send->onCancel = _onCancel;
    send->args = args;
    ui::newThread(confirm_t, send, confirmDrawFunc);
}

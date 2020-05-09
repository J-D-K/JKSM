#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>

#include "backupmenu.h"
#include "global.h"
#include "titledata.h"
#include "ui.h"
#include "menu.h"
#include "util.h"

enum backOpts
{
    saveDat,
    extDat,
    back,
};

static menu backMenu(0, 96, false, true);

void prepBackMenu()
{
    backMenu.addItem("Save Data Options");
    backMenu.addItem("Extra Data Options");
    backMenu.addItem("Back");

    backMenu.autoVert();
}

void backupMenu()
{
    if(curTitle == NULL)
    {
        showMessage("OH SHIT! THIS SHOULDN'T HAPPEN!", "curTitle is NULL!!!");
        state = prevState;
        return;
    }

    std::u32string info = curTitle->u32Name;
    if(curTitle->media == MEDIATYPE_GAME_CARD)
        info += U" : Cart";
    else
        info += U" : SD/CIA";

    hidScanInput();
    u32 down = hidKeysDown();

    backMenu.handleInput(down, 0);

    if(down & KEY_A)
    {
        switch(backMenu.getSelected())
        {
            case backOpts::saveDat:
                state = STATE_SAVEMENU;
                break;
            case backOpts::extDat:
                state = STATE_EXTMENU;
                break;
            case backOpts::back:
                state = prevState;
                break;
        }
    }
    else if(down & KEY_B)
        state = prevState;

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(info);
    backMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();
}

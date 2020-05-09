#include <3ds.h>
#include <stdio.h>
#include <string>

#include "dev.h"
#include "menu.h"
#include "util.h"
#include "global.h"
#include "ui.h"

//This is just for testing stuff.

static menu devMenu(0, 0, false, true);

void prepDevMenu()
{
    devMenu.addItem("fsStart");
    devMenu.addItem("fsEnd");
    devMenu.addItem("Back");

    devMenu.autoVert();
}

enum devOpts
{
    _fsStart,
    _fsEnd,
    _back
};

void showDevMenu()
{
    hidScanInput();

    u32 down = hidKeysDown();

    devMenu.handleInput(down, 0);

    if(down & KEY_A)
    {
        switch(devMenu.getSelected())
        {
            case devOpts::_fsStart:
                fsStart();
                break;
            case devOpts::_fsEnd:
                fsEnd();
                break;
            case devOpts::_back:
                state = STATE_MAINMENU;
                break;
        }
    }
    else if(down & KEY_B)
        state = STATE_MAINMENU;

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
        drawTopBar(U"DevMenu");
        devMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();
}

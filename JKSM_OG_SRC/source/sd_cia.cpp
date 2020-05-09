#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "titles.h"
#include "menu.h"
#include "global.h"
#include "util.h"
#include "ui.h"
#include "backup.h"
#include "restore.h"

const std::string helpText = "Press L to select multiple. Press R to select all. Press Y to backup selected. Press X to restore selected.";

//Help button
static button help("Help", 224, 208, 96, 32);

static menu titleMenu(40, 20, true, false);

void prepSDSelect()
{
    titleMenu.reset();
    for(unsigned i = 0; i < sdTitle.size(); i++)
        titleMenu.addItem(sdTitle[i].name);
    if(centered)
        titleMenu.centerOpts();

    titleMenu.autoVert();
}

void cartCheck()
{
    bool ins = false;
    FSUSER_CardSlotIsInserted(&ins);
    if(ins && sdTitle[0].media == MEDIATYPE_SD)
    {
        uint64_t cardId = 0;
        AM_GetTitleList(NULL, MEDIATYPE_GAME_CARD, 1, &cardId);
        titleData cardData;
        if(cardData.init(cardId, MEDIATYPE_GAME_CARD))
        {
            sdTitle.insert(sdTitle.begin(), cardData);
            prepSDSelect();
        }
    }
    else if(!ins && sdTitle[0].media == MEDIATYPE_GAME_CARD)
    {
        sdTitle.erase(sdTitle.begin(), sdTitle.begin() + 1);
        prepSDSelect();
    }
}

void sdStartSelect()
{
    cartCheck();
    if(sdTitle.size() < 1)
    {
        showMessage("No installed titles were found!", "Nope...");
        state = states::STATE_MAINMENU;
        return;
    }

    hidScanInput();

    u32 down = hidKeysDown();
    u32 held = hidKeysHeld();

    titleMenu.handleInput(down, held);

    touchPosition pos;
    hidTouchRead(&pos);

    if(down & KEY_A)
    {
        unsigned selected = titleMenu.getSelected();
        curTitle = &sdTitle[selected];
        renameDir(sdTitle[selected]);
        prevState = states::STATE_TITLEMENU;
        state = states::STATE_BACKUPMENU;
    }
    else if(help.released(pos))
    {
        showMessage(helpText.c_str(), "Help");
    }
    else if(down & KEY_Y)
    {
        autoBackup(titleMenu);
    }
    else if(down & KEY_X)
    {
        autoRestore(titleMenu);
    }
    else if(down & KEY_B)
        state = states::STATE_MAINMENU;

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(U"Select a title");
    titleMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sdTitle[titleMenu.getSelected()].printInfo();
    help.draw();
    sf2d_end_frame();

    sf2d_swapbuffers();
}

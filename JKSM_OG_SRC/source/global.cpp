#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <stdio.h>
#include <string>
#include <string.h>

#include "global.h"
#include "date.h"
#include "sys.h"
#include "menu.h"
#include "sd_cia.h"
#include "titledata.h"
#include "backupmenu.h"
#include "savemenu.h"
#include "extmenu.h"
#include "titles.h"
#include "nand.h"
#include "hbfilter.h"
#include "extra.h"
#include "shared.h"
#include "dev.h"
#include "ui.h"

unsigned const buff_size = 0x20000;

sftd_font * font;

FS_Archive sdArch;

bool hbl = false, devMode = false, kill = false;

//config
bool centered = true, autoBack = false, useLang = false;

//default colors
u8 clearColor[3] = {0, 0, 0};
u8 selColor[3] = {0, 255, 0};
u8 unSelColor[3] = {128, 128, 128};

//I needed a quick way to get most of it under one loop without having to completely rewrite it
//This is what I came up with.
int state = states::STATE_MAINMENU, prevState = states::STATE_MAINMENU;
u8 sysLanguage = 1;
titleData *curTitle = NULL;

void handleState()
{
    switch(state)
    {
        case states::STATE_MAINMENU:
            mainMenu();
            break;
        case states::STATE_TITLEMENU:
            sdStartSelect();
            break;
        case states::STATE_BACKUPMENU:
            backupMenu();
            break;
        case states::STATE_SAVEMENU:
            showSaveMenu();
            break;
        case states::STATE_EXTMENU:
            showExtMenu();
            break;
        case states::STATE_NANDSELECT:
            nandStartSelect();
            break;
        case states::STATE_NANDBACKUP:
            nandBackup();
            break;
        case states::STATE_EXTRAS:
            extrasMenu();
            break;
        case states::STATE_SHARED:
            sharedExtManager();
            break;
        case states::STATE_SHAREDBACKUP:
            sharedBackupMenu();
            break;
        case states::STATE_DEVMENU:
            showDevMenu();
            break;
        default:
            showMessage("This shouldn't happen.", "Umm...");
            state = states::STATE_MAINMENU;
            break;
    }
}

void killApp(u32 up)
{
    if(up & KEY_START)
        kill = true;
}

enum mMenuOpts
{
    cia,
    sys,
    shared,
    ref,
    filter,
    extra,
    exitapp,
    dev
};

static menu mMenu(136, 80, false, true);

void prepMain()
{
    mMenu.addItem("Titles");
    mMenu.addItem("System Titles");
    mMenu.addItem("Shared ExtData");
    mMenu.addItem("Refresh Games");
    mMenu.addItem("Download Filter");
    mMenu.addItem("Config/Extras");
    mMenu.addItem("Exit");

    if(devMode)
        mMenu.addItem("Dev");

    mMenu.autoVert();
}

void mainMenu()
{

    hidScanInput();

    u32 down = hidKeysDown();

    mMenu.handleInput(down, 0);
    if(down & KEY_A)
    {
        switch(mMenu.getSelected())
        {
            case mMenuOpts::cia:
                state = states::STATE_TITLEMENU;
                break;
            case mMenuOpts::sys:
                state = states::STATE_NANDSELECT;
                break;
            case mMenuOpts::shared:
                state = states::STATE_SHARED;
                break;
            case mMenuOpts::ref:
                remove("titles");
                sdTitlesInit();
                break;
            case mMenuOpts::filter:
                remove("filter.txt");
                loadFilterList();
                break;
            case mMenuOpts::extra:
                state = STATE_EXTRAS;
                break;
            case mMenuOpts::exitapp:
                kill = true;
                break;
            case mMenuOpts::dev:
                state = STATE_DEVMENU;
                break;
        }
    }
    else if(down & KEY_B)
        kill = true;

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(std::u32string(U"JKSM - " + BUILD_DATE));
    mMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();
}

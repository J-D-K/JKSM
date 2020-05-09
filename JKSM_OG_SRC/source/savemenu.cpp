#include <3ds.h>
#include <stdio.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>

#include "savemenu.h"
#include "archive.h"
#include "backup.h"
#include "restore.h"
#include "menu.h"
#include "titledata.h"
#include "global.h"
#include "ui.h"
#include "util.h"

#include "file.h"

enum saveOpts
{
    expSav,
    impSav,
    browseSav,
    delSV,
    delSav,
    back,
    fun
};

static menu saveMenu(128, 80, false, true);

void prepSaveMenu()
{
    saveMenu.addItem("Export Save");
    saveMenu.addItem("Import Save");
    saveMenu.addItem("Browse SD for Data");
    saveMenu.addItem("Delete Secure Value");
    saveMenu.addItem("Delete Save Data");
    saveMenu.addItem("Back");

    saveMenu.autoVert();
}

void showSaveMenu()
{
    std::u32string info = curTitle->u32Name + U" : Save Data";

    hidScanInput();

    u32 down = hidKeysDown();

    touchPosition p;
    hidTouchRead(&p);

    saveMenu.handleInput(down, 0);

    if(down & KEY_A)
    {
        FS_Archive saveArch;
        switch(saveMenu.getSelected())
        {
            case saveOpts::expSav:
                if(openSaveArch(&saveArch, *curTitle, true))
                {
                    createTitleDir(*curTitle, MODE_SAVE);
                    backupData(*curTitle, saveArch, MODE_SAVE, false);
                }
                break;
            case saveOpts::impSav:
                if(openSaveArch(&saveArch, *curTitle, true))
                    restoreData(*curTitle, saveArch, MODE_SAVE);
                break;
            case saveOpts::browseSav:
                if(openSaveArch(&saveArch, *curTitle, true))
                    restoreDataSDPath(*curTitle, saveArch, MODE_SAVE);
                break;
            case saveOpts::delSV:
                if(deleteSV(*curTitle))
                    showMessage("Secure value successfully deleted!", "Success!");
                break;
            case saveOpts::delSav:
                if(openSaveArch(&saveArch, *curTitle, true) && confirm("Are you sure you want to delete this title's current save data?"))
                {
                    FSUSER_DeleteDirectoryRecursively(saveArch, fsMakePath(PATH_ASCII, "/"));
                    FSUSER_ControlArchive(saveArch, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
                }
                break;
            case saveOpts::back:
                state = STATE_BACKUPMENU;
                break;
        }
        FSUSER_CloseArchive(saveArch);
    }
    else if(down & KEY_B)
    {
        state = STATE_BACKUPMENU;
    }

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(info);
    saveMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();
}

#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>

#include "extmenu.h"
#include "archive.h"
#include "titledata.h"
#include "backup.h"
#include "restore.h"
#include "global.h"
#include "ui.h"
#include "util.h"
#include "menu.h"

enum extOpts
{
    expExt,
    impExt,
    browseExt,
    delExt,
    back
};

static menu extMenu(128, 80, false, true);

void prepExtMenu()
{
    extMenu.addItem("Export ExtData");
    extMenu.addItem("Import ExtData");
    extMenu.addItem("Browse for ExtData");
    extMenu.addItem("Delete ExtData");
    extMenu.addItem("Back");

    extMenu.autoVert();
}

void showExtMenu()
{
    std::u32string info = curTitle->u32Name + U" : Extra Data";

    hidScanInput();

    u32 down = hidKeysDown();

    touchPosition p;
    hidTouchRead(&p);

    extMenu.handleInput(down, 0);

    killApp(down);

    if(down & KEY_A)
    {
        FS_Archive extData;
        switch(extMenu.getSelected())
        {
            case extOpts::expExt:
                if(openExtdata(&extData, *curTitle, true))
                {
                    createTitleDir(*curTitle, MODE_EXTDATA);
                    backupData(*curTitle, extData, MODE_EXTDATA, false);
                }
                break;
            case extOpts::impExt:
                if(openExtdata(&extData, *curTitle, true))
                    restoreData(*curTitle, extData, MODE_EXTDATA);
                else
                {
                    if(confirm("Would you like to try to create Extra Data for this title?"))
                        createExtData(*curTitle);
                }
                break;
            case extOpts::browseExt:
                if(openExtdata(&extData, *curTitle, true))
                    restoreDataSDPath(*curTitle, extData, MODE_EXTDATA);
                else
                {
                    if(confirm("Would you like to try to create Extra Data for this title?"))
                        createExtData(*curTitle);
                }
                break;
            case extOpts::delExt:
                if(confirm("Are you sure you want to delete this title's extdata?"))
                    deleteExtdata(*curTitle);
                break;
            case extOpts::back:
                state = STATE_BACKUPMENU;
                break;
        }
        FSUSER_CloseArchive(extData);
    }

    else if(down & KEY_B)
        state = STATE_BACKUPMENU;

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(info);
    extMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();
}

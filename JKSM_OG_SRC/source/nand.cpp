#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <string.h>
#include <string>

#include "titles.h"
#include "nand.h"
#include "backup.h"
#include "restore.h"
#include "util.h"
#include "ui.h"
#include "global.h"
#include "menu.h"
#include "titledata.h"
#include "archive.h"

titleData * sysTitle = NULL;

static menu nandMenu(40, 20, false, false);

void prepNandSelect()
{
    for(unsigned i = 0; i < nandTitle.size(); i++)
        nandMenu.addItem(nandTitle[i].name);
    if(centered)
        nandMenu.centerOpts();

    nandMenu.autoVert();
}

void nandStartSelect()
{
    hidScanInput();

    u32 down = hidKeysDown();
    u32 held = hidKeysHeld();

    nandMenu.handleInput(down, held);

    if(down & KEY_A)
    {
        sysTitle = &nandTitle[nandMenu.getSelected()];
        state = STATE_NANDBACKUP;
    }
    else if(down & KEY_B)
        state = STATE_MAINMENU;

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    nandMenu.draw();
    drawTopBar(U"Select System Title");
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    nandTitle[nandMenu.getSelected()].printInfo();
    sf2d_end_frame();

    sf2d_swapbuffers();
}

enum sysOpts
{
    expSys,
    impSys,
    expExt,
    impExt,
    expBoss,
    impBoss,
    back
};

static menu nBackupMenu(112, 72, false, true);

void prepNandBackup()
{
    nBackupMenu.addItem("Export System Save");
    nBackupMenu.addItem("Import System Save");
    nBackupMenu.addItem("Export ExtData");
    nBackupMenu.addItem("Import ExtData");
    nBackupMenu.addItem("Export Boss ExtData");
    nBackupMenu.addItem("Import Boss ExtData");
    nBackupMenu.addItem("Back");

    nBackupMenu.autoVert();
}

void nandBackup()
{
    std::u32string info = tou32(sysTitle->name) + U" : NAND";

    hidScanInput();

    u32 down = hidKeysDown();

    nBackupMenu.handleInput(down, 0);

    if(down & KEY_A)
    {
        FS_Archive arch;
        switch(nBackupMenu.getSelected())
        {
            case sysOpts::expSys:
                if(openSysSave(&arch, *sysTitle))
                {
                    renameDir(*sysTitle);
                    createTitleDir(*sysTitle, MODE_SYSSAVE);
                    backupData(*sysTitle, arch, MODE_SYSSAVE, false);
                }
                break;
            case sysOpts::impSys:
                if(openSysSave(&arch, *sysTitle))
                {
                    restoreData(*sysTitle, arch, MODE_SYSSAVE);
                }
                break;
            case sysOpts::expExt:
                if(openExtdata(&arch, *sysTitle, true))
                {
                    renameDir(*sysTitle);
                    createTitleDir(*sysTitle, MODE_EXTDATA);
                    backupData(*sysTitle, arch, MODE_EXTDATA, false);
                }
                break;
            case sysOpts::impExt:
                if(openExtdata(&arch, *sysTitle, true))
                {
                    restoreData(*sysTitle, arch, MODE_EXTDATA);
                }
                break;
            case sysOpts::expBoss:
                if(openBossExt(&arch, *sysTitle))
                {
                    renameDir(*sysTitle);
                    createTitleDir(*sysTitle, MODE_BOSS);
                    backupData(*sysTitle, arch, MODE_BOSS, false);
                }
                break;
            case sysOpts::impBoss:
                if(openBossExt(&arch, *sysTitle))
                {
                    restoreData(*sysTitle, arch, MODE_BOSS);
                }
                break;
            case sysOpts::back:
                state = states::STATE_NANDSELECT;
                break;
        }
        FSUSER_CloseArchive(arch);
    }
    else if(down & KEY_B)
        state = states::STATE_NANDSELECT;

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(info);
    nBackupMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();


}

#include <3ds.h>
#include <string>

#include "shared.h"
#include "archive.h"
#include "backup.h"
#include "restore.h"
#include "global.h"
#include "menu.h"
#include "util.h"
#include "ui.h"

//I stole these from 3dbrew. Kill me.
std::string descs[] =
{
    "From 3DBrew.org: Home Menu attempts to open this archive during boot, if FS:OpenArchive doesn't return an error Home Menu seems to then launch the System Transfer application. Home Menu doesn't actually use this archive at all except for checking whether it exists.",
    "From 3DBrew.org: NAND JPEG/MPO files and phtcache.bin from the camera application are stored here. This also contains UploadData.dat. ",
    "From 3DBrew.org: NAND M4A files from the sound application are stored here ",
    "From 3DBrew.org: ???",
    "From 3DBrew.org: Contains idb.dat, idbt.dat, gamecoin.dat, ubll.lst, CFL_DB.dat, and CFL_OldDB.dat. These files contain cleartext Miis and some data relating (including cached ICN data) to Play/Usage Records.",
    "From 3DBrew.org: Contains bashotorya.dat and bashotorya2.dat.",
    "From 3DBrew.org: ???",
    "From 3DBrew.org: Contains versionlist.dat, used by Home Menu for the software update notification added with 7.0.0-13.",
    "This means go back."
};

enum sharedOpts
{
    e0,
    f1,
    f2,
    f9,
    fb,
    fc,
    fd,
    fe
};

enum sharedBack
{
    _exp,
    _imp,
    _back
};

titleData selShared;

static menu sharedBackMenu(136, 96, false, true);

void prepSharedBackMenu()
{
    sharedBackMenu.addItem("Export Data");
    sharedBackMenu.addItem("Import Data");
    sharedBackMenu.addItem("Back");

    sharedBackMenu.autoVert();
}

void sharedBackupMenu()
{
    std::u32string info = tou32(selShared.nameSafe) + U" : Shared Extdata";

    hidScanInput();

    u32 down = hidKeysDown();

    sharedBackMenu.handleInput(down, 0);

    if(down & KEY_A)
    {
        FS_Archive arch;
        switch(sharedBackMenu.getSelected())
        {
            case sharedBack::_exp:
                if(openSharedExt(&arch, selShared.extdata))
                {
                    createTitleDir(selShared, MODE_SHARED);
                    backupData(selShared, arch, MODE_SHARED, false);
                }
                break;
            case sharedBack::_imp:
                if(openSharedExt(&arch, selShared.extdata))
                    restoreData(selShared, arch, MODE_SHARED);
                break;
            case sharedBack::_back:
                state = states::STATE_SHARED;
                break;
        }
        FSUSER_CloseArchive(arch);
    }
    else if(down & KEY_B)
        state = states::STATE_SHARED;

    //oops forgot this last time
    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(info);
    sharedBackMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();
}

static menu sharedMenu(128, 72, false, true);

void prepSharedMenu()
{
    sharedMenu.addItem("E0000000");
    sharedMenu.addItem("F0000001");
    sharedMenu.addItem("F0000002");
    sharedMenu.addItem("F0000009");
    sharedMenu.addItem("F000000B");
    sharedMenu.addItem("F000000C");
    sharedMenu.addItem("F000000D");
    sharedMenu.addItem("F000000E");

    sharedMenu.autoVert();
}

void sharedExtManager()
{
    std::u32string info = U"Shared ExtData";

    hidScanInput();

    u32 down = hidKeysDown();
    u32 held = hidKeysHeld();

    sharedMenu.handleInput(down, held);

    if(down & KEY_A)
    {
        titleData sharedDat;
        switch(sharedMenu.getSelected())
        {
            case sharedOpts::e0:
                sharedDat.extdata = 0xE0000000;
                sharedDat.nameSafe = tou16("E0000000");
                break;
            case sharedOpts::f1:
                sharedDat.extdata =  0xF0000001;
                sharedDat.nameSafe = tou16("F0000001");
                break;
            case sharedOpts::f2:
                sharedDat.extdata = 0xF0000002;
                sharedDat.nameSafe = tou16("F0000002");
                break;
            case sharedOpts::f9:
                sharedDat.extdata = 0xF0000009;
                sharedDat.nameSafe = tou16("F0000009");
                break;
            case sharedOpts::fb:
                sharedDat.extdata = 0xF000000B;
                sharedDat.nameSafe = tou16("F000000B");
                break;
            case sharedOpts::fc:
                sharedDat.extdata = 0xF000000C;
                sharedDat.nameSafe = tou16("F000000C");
                break;
            case sharedOpts::fd:
                sharedDat.extdata = 0xF000000D;
                sharedDat.nameSafe = tou16("F000000D");
                break;
            case sharedOpts::fe:
                sharedDat.extdata = 0xF000000E;
                sharedDat.nameSafe = tou16("F000000E");
                break;
        }
        selShared = sharedDat;
        state = states::STATE_SHAREDBACKUP;
    }
    else if(down & KEY_B)
        state = states::STATE_MAINMENU;

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(info);
    sharedMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sftd_draw_text_wrap(font, 0, 0, RGBA8(255, 255, 255, 255), 12, 320, descs[sharedMenu.getSelected()].c_str());
    sf2d_end_frame();

    sf2d_swapbuffers();
}

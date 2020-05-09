#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <stdio.h>
#include <string>

#include "archive.h"
#include "backup.h"
#include "menu.h"
#include "restore.h"
#include "titledata.h"
#include "global.h"
#include "util.h"
#include "ui.h"
#include "extra.h"

enum hblOpts
{
    expSav,
    impSav,
    delSV,
    expExt,
    impExt,
    extra,
    exitapp
};

void start3dsxMode()
{
    u64 id;
    APT_GetProgramID(&id);

    fsStart();
    FS_MediaType getMedia;
    FSUSER_GetMediaType(&getMedia);
    fsEnd();

    //This doesn't work if you start the FS session.
    titleData data;
    data.init(id, getMedia);

    renameDir(data);

    menu hblMenu(0, 80, false, true);
    hblMenu.addItem("Export Save");
    hblMenu.addItem("Import Save");
    hblMenu.addItem("Delete Secure Value");
    hblMenu.addItem("Export ExtData");
    hblMenu.addItem("Import ExtData");
    hblMenu.addItem("Extras/Config");
    hblMenu.addItem("Exit");

    std::u32string info = data.u32Name + U" : 3DSX Mode - " + BUILD_DATE;

    fsStart();

    while(aptMainLoop() && !kill)
    {
        hidScanInput();

        u32 down = hidKeysDown();

        touchPosition p;
        hidTouchRead(&p);

        hblMenu.handleInput(down, 0);

        if(down & KEY_A)
        {
            FS_Archive arch;
            switch(hblMenu.getSelected())
            {
                case hblOpts::expSav:
                    if(openSaveArch3dsx(&arch))
                    {
                        createTitleDir(data, MODE_SAVE);
                        backupData(data, arch, MODE_SAVE, false);
                    }
                    break;
                case hblOpts::impSav:
                    if(openSaveArch3dsx(&arch))
                        restoreData(data, arch, MODE_SAVE);
                    break;
                case hblOpts::delSV:
                    fsEnd();
                    if(deleteSV(data))
                        showMessage("Secure value successfully deleted!", "Success!");
                    fsStart();
                    break;
                case hblOpts::expExt:
                    if(openExtdata(&arch, data, true))
                    {
                        createTitleDir(data, MODE_EXTDATA);
                        backupData(data, arch, MODE_EXTDATA, false);
                    }
                    break;
                case hblOpts::impExt:
                    if(openExtdata(&arch, data, false))
                        restoreData(data, arch, MODE_EXTDATA);
                    break;
                case hblOpts::extra:
                    while(1)
                    {
                        extrasMenu();

                        if(hidKeysDown() & KEY_B)
                            break;

                    }
                    break;
                case hblOpts::exitapp:
                    kill = true;
                    break;
            }
            FSUSER_CloseArchive(arch);
        }

        killApp(down);

        sf2d_start_frame(GFX_TOP, GFX_LEFT);
        drawTopBar(info);
        hblMenu.draw();
        sf2d_end_frame();

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        data.printInfo();
        sf2d_end_frame();

        sf2d_swapbuffers();
    }

    fsEnd();
}

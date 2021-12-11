#include <3ds.h>

#include "ui.h"
#include "data.h"
#include "fs.h"
#include "util.h"

static ui::titleview *bossView;
static bool fldOpen = false;

static void fldCallback(void *)
{
    switch(ui::padKeysDown())
    {
        case KEY_B:
            fs::closeSaveArch();
            fldOpen = false;
            break;
    }
}

static void bossViewCallback(void *a)
{
    switch(ui::padKeysDown())
    {
        case KEY_A:
            {
                data::titleData *t = &data::bossDataTitles[bossView->getSelected()];
                std::string uploadParent;
                if(fs::gDrive)
                {
                    std::string ttlUTF8 = t->getTitleUTF8();
                    if(!fs::gDrive->dirExists(ttlUTF8, fs::bossExtDirID))
                        fs::gDrive->createDir(ttlUTF8, fs::bossExtDirID);

                    uploadParent = fs::gDrive->getFolderID(ttlUTF8, fs::bossExtDirID);
                }

                if(fs::openArchive(*t, ARCHIVE_BOSS_EXTDATA, false))
                {
                    util::createTitleDir(*t, ARCHIVE_BOSS_EXTDATA);
                    std::u16string targetDir = util::createPath(*t, ARCHIVE_BOSS_EXTDATA);
                    ui::fldInit(targetDir, uploadParent, fldCallback, NULL);
                    fldOpen = true;
                }
            }
            break;

        case KEY_CPAD_LEFT:
            ui::state = SYS;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = SHR;
            break;
    }
}

void ui::bossViewInit(void *a)
{
    threadInfo *t = (threadInfo *)a;
    bossView = new ui::titleview(data::bossDataTitles, bossViewCallback, NULL);
    t->finished = true;
}

void ui::bossViewExit()
{
    delete bossView;
}

void ui::bossViewUpdate()
{
    if(fldOpen)
        ui::fldUpdate();
    else
        bossView->update();
}

void ui::bossViewDrawTop()
{
    bossView->draw();
    ui::drawUIBar(TITLE_TEXT + "- BOSS Extra Data", ui::SCREEN_TOP, true);
}

void ui::bossViewDrawBot()
{
    if(fldOpen)
    {
        ui::fldDraw();
        ui::drawUIBar(FLD_GUIDE_TEXT, ui::SCREEN_BOT, true);
    }
    else
    {
        data::bossDataTitles[bossView->getSelected()].drawInfo(0, 0);
        ui::drawUIBar("\ue000 Open \ue01A\ue077\ue019 Save Type", ui::SCREEN_BOT, false);
    }
}

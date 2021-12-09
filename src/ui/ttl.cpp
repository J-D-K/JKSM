#include <3ds.h>

#include "ui.h"
#include "ttl.h"
#include "gfx.h"
#include "fs.h"
#include "util.h"

static ui::titleview *ttlView;
static ui::menu *ttlOpts;
static bool fldOpen = false, ttlOptsOpen = false;

static int findTitleNewIndex(const uint64_t& tid)
{
    for(unsigned i = 0; i < data::usrSaveTitles.size(); i++)
    {
        if(data::usrSaveTitles[i].getID() == tid)
            return i;
    }
    return 0;
}

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

static void ttlViewCallback(void *a)
{
    switch(ui::padKeysDown())
    {
        case KEY_SELECT:
            ui::confirm("Confirm?", NULL, NULL, NULL);
            break;

        case KEY_A:
            {
                data::titleData *t = &data::usrSaveTitles[ttlView->getSelected()];
                //For now for compatibility with old code
                data::curData = *t;
                if(fs::openArchive(*t, ARCHIVE_USER_SAVEDATA, false))
                {
                    util::createTitleDir(*t, ARCHIVE_USER_SAVEDATA);
                    std::u16string targetPath = util::createPath(*t, ARCHIVE_USER_SAVEDATA);
                    ui::fldInit(targetPath, fldCallback, NULL);
                    fldOpen = true;
                }
            }
            break;

        case KEY_X:
            {
                data::titleData *t = &data::usrSaveTitles[ttlView->getSelected()];
                data::curData = *t;
                ttlOptsOpen = true;
            }
            break;

        case KEY_Y:
            {
                data::titleData *t = &data::usrSaveTitles[ttlView->getSelected()];
                uint64_t tid = t->getID();
                if(t->getFav())
                    data::favRem(*t);
                else
                    data::favAdd(*t);
                ttlView->refesh(data::usrSaveTitles);
                int newSel = findTitleNewIndex(tid);
                ttlView->setSelected(newSel);
            }
            break;

        case KEY_CPAD_LEFT:
            ui::state = SET;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = EXT;
            break;
    }
}

static void ttlOptCallback(void *a)
{
    uint32_t down = ui::padKeysDown();
    switch (down)
    {
        case KEY_B:
            ttlOptsOpen = false;
            break;
    }
}

static void ttlOptResetSaveData_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    if(fs::openArchive(data::curData, fs::getSaveMode(), false))
    {
        t->status->setStatus("Resetting save data...");
        fs::delDirRec(fs::getSaveArch(), util::toUtf16("/"));
        fs::commitData(fs::getSaveMode());
        fs::closeSaveArch();
    }
    t->finished = true;
}

static void ttlOptResetSaveData(void *a)
{
    data::titleData *t = &data::usrSaveTitles[ttlView->getSelected()];
    std::string q = "Are you sure would like to reset the save data for " + util::toUtf8(t->getTitle()) + "?";
    ui::confirm(q, ttlOptResetSaveData_t, NULL, NULL);
}

void ui::ttlInit(void *a)
{
    threadInfo *t = (threadInfo *)a;
    ttlView = new titleview(data::usrSaveTitles, ttlViewCallback, NULL);
    ui::state = USR;

    ttlOpts = new ui::menu;
    ttlOpts->setCallback(ttlOptCallback, NULL);
    ttlOpts->addOpt("Reset Save Data", 320);
    ttlOpts->addOptEvent(0, KEY_A, ttlOptResetSaveData, NULL);

    t->finished = true;
}

void ui::ttlExit()
{
    delete ttlView;
    delete ttlOpts;
}

void ui::ttlRefresh()
{
    ttlView->refesh(data::usrSaveTitles);
}

void ui::ttlUpdate()
{
    if(fldOpen)
        fldUpdate();
    else if(ttlOptsOpen)
        ttlOpts->update();
    else
        ttlView->update();
}

void ui::ttlDrawTop()
{
    ttlView->draw();
    ui::drawUIBar(TITLE_TEXT + "- User Saves", ui::SCREEN_TOP, true);
}

void ui::ttlDrawBot()
{
    if(fldOpen)
    {
        ui::fldDraw();
        ui::drawUIBar(FLD_GUIDE_TEXT, ui::SCREEN_BOT, true);
    }
    else if(ttlOptsOpen)
    {
        ttlOpts->draw(0, 2, 0xFFFFFFFF, 320, false);
        ui::drawUIBar("\ue000 Select \ue001 Close", ui::SCREEN_BOT, false);
    }
    else
    {
        data::usrSaveTitles[ttlView->getSelected()].drawInfo(0, 0);
        ui::drawUIBar("\ue000 Open \ue002 Options \ue003 Favorite \ue01A\ue077\ue019 Save Type", ui::SCREEN_BOT, true);
    }
}

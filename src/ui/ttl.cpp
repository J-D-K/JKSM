#include <3ds.h>

#include "ui.h"
#include "ttl.h"
#include "gfx.h"
#include "fs.h"
#include "util.h"

static ui::titleview *ttlView;
static bool fldOpen = false;

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

void ui::ttlInit()
{
    ttlView = new titleview(data::usrSaveTitles, ttlViewCallback, NULL);
    ui::state = USR;
}

void ui::ttlExit()
{
    delete ttlView;
}

void ui::ttlRefresh()
{
    ttlView->refesh(data::usrSaveTitles);
}

void ui::ttlUpdate()
{
    if(!fldOpen)
        ttlView->update();
    else
        fldUpdate();
}

void ui::ttlDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- User Saves", ui::SCREEN_TOP, true);
    ttlView->draw();
}

void ui::ttlDrawBot()
{
    if(fldOpen)
        ui::fldDraw();

    ui::drawUIBar("Select a title", ui::SCREEN_BOT, true);
}

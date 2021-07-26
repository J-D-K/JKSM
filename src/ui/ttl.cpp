#include <3ds.h>

#include "ui.h"
#include "ttl.h"
#include "gfx.h"
#include "fs.h"
#include "util.h"

static ui::titleview *ttlView;

static int findTitleNewIndex(const uint64_t& tid)
{
    for(unsigned i = 0; i < data::usrSaveTitles.size(); i++)
    {
        if(data::usrSaveTitles[i].getID() == tid)
            return i;
    }
    return 0;
}

void ttlViewCallback(void *a)
{
    switch(ui::padKeysDown())
    {
        case KEY_A:
            {
                data::titleData *t = &data::usrSaveTitles[ttlView->getSelected()];
                if(fs::openArchive(*t, ARCHIVE_USER_SAVEDATA, false))
                {
                    util::createTitleDir(*t, ARCHIVE_USER_SAVEDATA);
                    std::u16string from = util::toUtf16("/");
                    std::u16string to = util::createPath(*t, ARCHIVE_USER_SAVEDATA);
                    fs::copyDirToSD(fs::getSaveArch(), from, to);
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
            ui::state = SHR;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = EXT;
            break;
    }
}

void ui::ttlInit(void *a)
{
    threadInfo *t = (threadInfo *)a;
    t->status->setStatus("Preparing User Save View...");
    ttlView = new titleview(data::usrSaveTitles, ttlViewCallback, NULL);
    ui::state = USR;
    t->finished = true;
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
    ttlView->update();
}

void ui::ttlDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- User Saves", ui::SCREEN_TOP, true);
    ttlView->draw();
}

void ui::ttlDrawBot()
{
    ui::drawUIBar("Select a title", ui::SCREEN_BOT, true);
}

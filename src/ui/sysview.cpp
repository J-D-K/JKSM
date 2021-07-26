#include <3ds.h>

#include "ui.h"
#include "data.h"
#include "fs.h"
#include "util.h"

static ui::titleview *sysView;

static void sysViewCallback(void *a)
{
    switch(ui::padKeysDown())
    {
        case KEY_A:
            {
                data::titleData *t = &data::sysDataTitles[sysView->getSelected()];
                if(fs::openArchive(*t, ARCHIVE_SYSTEM_SAVEDATA, false))
                {
                    util::createTitleDir(*t, ARCHIVE_SYSTEM_SAVEDATA);
                    std::u16string root = util::toUtf16("/");
                    std::u16string out  = util::createPath(*t, ARCHIVE_SYSTEM_SAVEDATA);
                    fs::copyDirToSD(fs::getSaveArch(), root, out);
                }
            }
            break;

        case KEY_CPAD_LEFT:
            ui::state = EXT;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = BOS;
            break;
    }
}

void ui::sysInit(void *a)
{
    threadInfo *t = (threadInfo *)a;
    t->status->setStatus("Preparing System Save View...");
    sysView = new ui::titleview(data::sysDataTitles, sysViewCallback, NULL);
    t->finished = true;
}

void ui::sysExit()
{
    delete sysView;
}

void ui::sysUpdate()
{
    sysView->update();
}

void ui::sysDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- System Saves", ui::SCREEN_TOP, true);
    sysView->draw();
}

void ui::sysDrawBot()
{
    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

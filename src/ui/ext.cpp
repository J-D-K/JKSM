#include <3ds.h>

#include "ui.h"
#include "data.h"
#include "fs.h"
#include "util.h"

static ui::titleview *extView;

static void extViewCallback(void *a)
{
    switch(ui::padKeysDown())
    {
        case KEY_A:
            {
                data::titleData *t = &data::extDataTitles[extView->getSelected()];
                if(fs::openArchive(*t, ARCHIVE_EXTDATA, false))
                {
                    std::u16string root = util::toUtf16("/");
                    std::u16string out  = util::createPath(*t, ARCHIVE_EXTDATA);
                    util::createTitleDir(*t, ARCHIVE_EXTDATA);
                    fs::copyDirToSD(fs::getSaveArch(), root, out);
                }
            }
            break;

        case KEY_CPAD_LEFT:
            ui::state = USR;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = SYS;
            break;
    }
}

void ui::extInit(void *a)
{
    threadInfo *t = (threadInfo *)a;
    t->status->setStatus("Preparing ExtData View...");
    extView = new ui::titleview(data::extDataTitles, extViewCallback, NULL);
    t->finished = true;
}

void ui::extExit()
{
    delete extView;
}

void ui::extUpdate()
{
    extView->update();
}

void ui::extRefresh()
{
    extView->refesh(data::extDataTitles);
}

void ui::extDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- Extra Data", ui::SCREEN_TOP, true);
    extView->draw();
}

void ui::extDrawBot()
{
    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

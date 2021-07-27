#include <3ds.h>

#include "ui.h"
#include "data.h"
#include "fs.h"
#include "util.h"

static ui::titleview *extView;
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

static void extViewCallback(void *a)
{
    switch(ui::padKeysDown())
    {
        case KEY_A:
            {
                data::titleData *t = &data::extDataTitles[extView->getSelected()];
                if(fs::openArchive(*t, ARCHIVE_EXTDATA, false))
                {
                    util::createTitleDir(*t, ARCHIVE_EXTDATA);
                    std::u16string targetPath = util::createPath(*t, ARCHIVE_EXTDATA);
                    ui::fldInit(targetPath, fldCallback, NULL);
                    fldOpen = true;
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
    if(fldOpen)
        ui::fldUpdate();
    else
        extView->update();
}

void ui::extRefresh(void *a)
{
    threadInfo *t = NULL;
    if(a)
    {
        t = (threadInfo *)a;
        t->status->setStatus("Updating ExtData View...");
    }
    extView->refesh(data::extDataTitles);
    if(t)
        t->finished = true;
}

void ui::extDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- Extra Data", ui::SCREEN_TOP, true);
    extView->draw();
}

void ui::extDrawBot()
{
    if(fldOpen)
        ui::fldDraw();

    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

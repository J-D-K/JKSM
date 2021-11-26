#include <3ds.h>

#include "ui.h"
#include "data.h"
#include "fs.h"
#include "util.h"

static ui::titleview *extView;
static bool fldOpen = false;

static void fldCallback(void *a)
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

void ui::extInit()
{
    extView = new ui::titleview(data::extDataTitles, extViewCallback, NULL);
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
    if(fldOpen)
        ui::fldDraw();

    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

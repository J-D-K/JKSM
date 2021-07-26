#include <3ds.h>

#include "ui.h"
#include "data.h"

static ui::titleview *bossView;

static void bossViewCallback(void *a)
{
    switch(ui::padKeysDown())
    {
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
    t->status->setStatus("Preparing BOSS View...");
    bossView = new ui::titleview(data::bossDataTitles, bossViewCallback, NULL);
    t->finished = true;
}

void ui::bossViewExit()
{
    delete bossView;
}

void ui::bossViewUpdate()
{
    bossView->update();
}

void ui::bossViewDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- BOSS Extra Data", ui::SCREEN_TOP, true);
    bossView->draw();
}

void ui::bossViewDrawBot()
{
    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

#include <3ds.h>
#include <vector>

#include "ui.h"
#include "data.h"
#include "util.h"
#include "gfx.h"

static ui::titleview *shrdView;
//These are hardcoded/specific
//Never change, don't need caching
static std::vector<data::titleData> shared;

static void shrdViewCallback(void *)
{
    switch(ui::padKeysDown())
    {
        case KEY_CPAD_LEFT:
            ui::state = BOS;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = USR;
            break;
    }
}

void ui::shrdInit(void *a)
{
    threadInfo *t = (threadInfo *)a;
    t->status->setStatus("Setting up Shared ExtData...");
    //Skip E0 since it doesn't open
    data::titleData shrdExt;
    shrdExt.setExtdata(0xF0000001);
    C2D_Image icon = util::createIconGeneric("F1", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF0000002);
    icon = util::createIconGeneric("F2", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF0000009);
    icon = util::createIconGeneric("F9", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000B);
    icon = util::createIconGeneric("FB", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000C);
    icon = util::createIconGeneric("FC", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000D);
    icon = util::createIconGeneric("FD", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000E);
    icon = util::createIconGeneric("FE", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdView = new titleview(shared, shrdViewCallback, NULL);
    t->finished = true;
}

void ui::shrdExit()
{
    for(data::titleData& d : shared)
        d.freeIcon();

    delete shrdView;
}

void ui::shrdUpdate()
{
    shrdView->update();
}

void ui::shrdDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- Shared ExtData", ui::SCREEN_TOP, true);
    shrdView->draw();
}

void ui::shrdDrawBot()
{
    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

#include <3ds.h>

#include "ui.h"
#include "cfg.h"
#include "fs.h"

static ui::menu setMenu;

static void toggleBool(void *b)
{
    bool *in = (bool *)b;
    if(*in)
        *in = false;
    else
        *in = true;
}

static std::string getBoolText(const bool& g)
{
    if(g)
        return "On";
    
    return "Off";
}

static void setMenuReloadTitles(void *a)
{
    remove("/JKSV/cache.bin");
    ui::newThread(data::loadTitles, NULL, NULL);
}

static void setMenuReloadDriveList_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    t->status->setStatus("Reloading Google Drive List...");
    fs::gDrive->loadDriveList();
    t->finished = true;
}

static void setMenuReloadDriveList(void *a)
{
    if(fs::gDrive)
        ui::newThread(setMenuReloadDriveList_t, NULL, NULL);
}

void ui::setInit(void *a)
{
    threadInfo *t = (threadInfo *)a;
    setMenu.addOpt("Reload Titles", 320);
    setMenu.addOptEvent(0, KEY_A, setMenuReloadTitles, NULL);

    setMenu.addOpt("Reload Google Drive List", 320);
    setMenu.addOptEvent(1, KEY_A, setMenuReloadDriveList, NULL);

    setMenu.addOpt("Export To ZIP", 320);
    setMenu.addOptEvent(2, KEY_A, toggleBool, &cfg::config["zip"]);
    t->finished = true;
}

void ui::setExit()
{

}

void ui::setUpdate()
{
    switch(ui::padKeysDown())
    {
        case KEY_CPAD_LEFT:
            ui::state = SHR;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = USR;
            break;
    }

    setMenu.editOpt(2, "Export to ZIP: " + getBoolText(cfg::config["zip"]));

    setMenu.update();
}

void ui::setDrawTop()
{
    setMenu.draw(0, 22, 0xFFFFFFFF, 400, false);
    ui::drawUIBar(TITLE_TEXT + "- Settings", ui::SCREEN_TOP, true);
}

void ui::setDrawBottom()
{
    ui::drawUIBar("\ue000 Select/Toggle \ue01A\ue077\ue019 Save Type", ui::SCREEN_BOT, false);
}
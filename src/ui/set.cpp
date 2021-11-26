#include <3ds.h>

#include "ui.h"
#include "cfg.h"

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

void ui::setInit()
{
    setMenu.addOpt("Export To ZIP", 320);
    setMenu.addOptEvent(0, KEY_A, toggleBool, &cfg::config["zip"]);
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

    setMenu.editOpt(0, "Export to ZIP: " + getBoolText(cfg::config["zip"]));

    setMenu.update();
}

void ui::setDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- Settings", ui::SCREEN_TOP, true);
    setMenu.draw(0, 22, 0xFFFFFFFF, 400, false);
}

void ui::setDrawBottom()
{
    ui::drawUIBar("", ui::SCREEN_BOT, false);
}
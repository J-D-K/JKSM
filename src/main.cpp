#include <3ds.h>

#include "gfx.h"
#include "fs.h"
#include "ui.h"
#include "util.h"
#include "data.h"
#include "sys.h"
#include "cfg.h"

//Needed for zip/minizip
u32 __stacksize__ = 0x100000;

extern int state;

int main(int argc, const char *argv[])
{
    sys::init();
    gfx::init();
    fs::init();
    data::loadTitles();
    ui::init();
    cfg::initToDefault();
    cfg::load();

    while(aptMainLoop() && ui::runApp()){ }

    cfg::save();
    data::saveFav();
    data::saveBlacklist();
    sys::exit();
    gfx::exit();
    fs::exit();
    data::exit();
    ui::exit();
}

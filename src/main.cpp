#include <3ds.h>

#include "gfx.h"
#include "fs.h"
#include "ui.h"
#include "util.h"
#include "data.h"
#include "sys.h"

extern int state;

int main(int argc, const char *argv[])
{
    sys::init();
    gfx::init();
    fs::init();
    ui::init();

    //Push these before main. Thread manager executes in order received
    ui::newThread(data::loadTitles, NULL, NULL);
    ui::newThread(ui::ttlInit, NULL, NULL);
    ui::newThread(ui::extInit, NULL, NULL);
    ui::newThread(ui::sysInit, NULL, NULL);
    ui::newThread(ui::bossViewInit, NULL, NULL);
    ui::newThread(ui::shrdInit, NULL, NULL);

    while(aptMainLoop() && ui::runApp()){ }

    data::saveFav();
    data::saveBlacklist();
    sys::exit();
    gfx::exit();
    fs::exit();
    data::exit();
    ui::exit();
}

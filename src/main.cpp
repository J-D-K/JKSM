#include <3ds.h>
#include <malloc.h>

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

static uint32_t *socBuffer;

int main(int argc, const char *argv[])
{
    sys::init();
    data::init();
    gfx::init();
    fs::init();
    cfg::initToDefault();
    cfg::load();
    ui::init();

    //Need to init soc so curl and drive work
    socBuffer = (uint32_t *)memalign(0x1000, 0x80000);
    socInit(socBuffer, 0x80000);

    curl_global_init(CURL_GLOBAL_ALL);

    ui::newThread(data::loadTitles, NULL, NULL);
    ui::newThread(ui::ttlInit, NULL, NULL);
    ui::newThread(ui::extInit, NULL, NULL);
    ui::newThread(ui::sysInit, NULL, NULL);
    ui::newThread(ui::bossViewInit, NULL, NULL);
    ui::newThread(ui::shrdInit, NULL, NULL);
    ui::newThread(ui::setInit, NULL, NULL);
    
    if(!cfg::driveClientID.empty() && !cfg::driveClientSecret.empty())
        ui::newThread(fs::driveInit, NULL, NULL);

    while(aptMainLoop() && ui::runApp()){ }

    curl_global_cleanup();

    cfg::save();
    data::saveFav();
    data::saveBlacklist();
    sys::exit();
    gfx::exit();
    fs::driveExit();
    fs::exit();
    data::exit();
    ui::exit();
    socExit();
}

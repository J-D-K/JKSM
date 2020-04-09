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
    data::loadTitles();
    data::loadNand();
    ui::prepMenus();

    while(aptMainLoop() && sys::run)
    {
        hidScanInput();

        uint32_t down = hidKeysDown();
        uint32_t held = hidKeysHeld();

        if(down & KEY_START)
            break;

        ui::runApp(down, held);
    }

    data::saveFav();
    sys::exit();
    gfx::exit();
    fs::exit();
}

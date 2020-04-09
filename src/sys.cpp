#include <3ds.h>
#include <cstdio>

#include "sys.h"

namespace sys
{
    bool run = true;
    uint8_t lang;

    void init()
    {
        hidInit();
        amInit();
        aptInit();
        cfguInit();
        romfsInit();
        CFGU_GetSystemLanguage(&lang);
    }

    void exit()
    {
        hidExit();
        amExit();
        aptExit();
        cfguExit();
        romfsExit();
    }
}

#include <3ds.h>
#include <cstdio>

#include "sys.h"

namespace sys
{
    bool run = true;
    void init()
    {
        hidInit();
        amInit();
        aptInit();
    }

    void exit()
    {
        hidExit();
        amExit();
        aptExit();
    }
}

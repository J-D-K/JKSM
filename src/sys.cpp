#include <3ds.h>
#include <cstdio>

#include "sys.h"

bool run = true;

void sys::init()
{
    hidInit();
    amInit();
    aptInit();
    romfsInit();
}

void sys::exit()
{
    hidExit();
    amExit();
    aptExit();
    romfsExit();
}

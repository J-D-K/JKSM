#include <3ds.h>
#include <cstdio>

#include "sys.h"

bool run = true;
int8_t sys::threadCore = -2;
bool sys::isNew3DS = false;
int sys::threadPrio = 0;

void sys::init()
{
    cfguInit();
    hidInit();
    amInit();
    aptInit();
    romfsInit();

    svcGetThreadPriority((s32 *)&sys::threadPrio, CUR_THREAD_HANDLE);
    --sys::threadPrio;

    uint8_t model = 0;
    CFGU_GetSystemModel(&model);
    if(model == 2 || model == 4 || model == 5)
    {
        sys::threadCore = 2;
        sys::isNew3DS = true;
    }
}

void sys::exit()
{
    cfguExit();
    hidExit();
    amExit();
    aptExit();
    romfsExit();
}

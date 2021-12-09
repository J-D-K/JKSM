#include <3ds.h>
#include <vector>

#include "ui.h"
#include "type.h"
#include "gfx.h"
#include "fs.h"
#include "util.h"

#include "thrdMgr.h"

/*Adapted the best I can from switch*/
ui::threadProcMngr::threadProcMngr()
{
    svcGetThreadPriority((s32 *)&prio, CUR_THREAD_HANDLE);
    prio -= 2;
    svcCreateMutex(&threadLock, false);
}

ui::threadProcMngr::~threadProcMngr()
{
    svcCloseHandle(threadLock);
    for(threadInfo *t : threads)
    {
        threadJoin(t->thrd, U64_MAX);
        threadFree(t->thrd);
        delete t->status;
        delete t;
    }
}

threadInfo *ui::threadProcMngr::newThread(ThreadFunc func, void *args, funcPtr _drawFunc)
{
    threadInfo *t = new threadInfo;
    t->status = new threadStatus;
    t->running = false;
    t->finished = false;
    t->thrdFunc = func;
    t->drawFunc = _drawFunc;
    t->argPtr = args;
    t->thrdID = cnt++;

    svcWaitSynchronization(threadLock, U64_MAX);
    threads.push_back(t);
    svcReleaseMutex(threadLock);

    return threads[threads.size() - 1];
}

void ui::threadProcMngr::update()
{
    if(!threads.empty())
    {
        threadInfo *t = threads[0];
        if(!t->running)
        {
            t->thrd = threadCreate(t->thrdFunc, t, 0x10000, prio, -2, false);
            t->running = true;
        }
        else if(t->finished)
        {
            threadJoin(t->thrd, U64_MAX);
            threadFree(t->thrd);
            delete t->status;
            delete t;
            svcWaitSynchronization(threadLock, U64_MAX);
            threads.erase(threads.begin());
            svcReleaseMutex(threadLock);
        }
    }
}

void ui::threadProcMngr::drawTop()
{
    C2D_DrawRectSolid(0, 0, GFX_DEPTH_DEFAULT, 400, 240, 0xBB000000);
}

void ui::threadProcMngr::drawBot()
{
    C2D_DrawRectSolid(0, 0, GFX_DEPTH_DEFAULT, 320, 240, 0xBB000000);

    if(++frameCount % 4 == 0 && ++lgFrame > 7)
        lgFrame = 0;

    if(clrAdd && (clrShft += 6) >= 0x72)
        clrAdd = false;
    else if(!clrAdd && (clrShft -= 3) <= 0x00)
        clrAdd = true;

    uint32_t glyphCol = 0xFF << 24 | (uint8_t)(0xC5 + (clrShft / 2)) << 16 | (uint8_t)(0x88 + clrShft) << 8 | 0x00;
    gfx::drawText(ui::loadGlyphArray[lgFrame], 4, 222, GFX_DEPTH_DEFAULT, 0.65f, glyphCol);
    
    threadInfo *t = threads[0];
    t->lock();
    if(t->running)
    {
        if(t->drawFunc)
            (*t->drawFunc)(t);
        else
        {
            std::string thrdstatus;
            t->status->getStatus(thrdstatus);
            int txtX = 160 - (gfx::getTextWidth(thrdstatus) / 2);

            gfx::drawText(thrdstatus, txtX, 114, GFX_DEPTH_DEFAULT, 0.5f, 0xFFFFFFFF);
        }
    }
    t->unlock();
}
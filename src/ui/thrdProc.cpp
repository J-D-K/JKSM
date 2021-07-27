#include <3ds.h>
#include <vector>

#include "ui.h"
#include "type.h"
#include "gfx.h"

#include "thrdProc.h"

/*Adapted the best I can from switch*/

ui::threadProcMngr::threadProcMngr()
{
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
            t->running = true;
            t->thrd = threadCreate(t->thrdFunc, t, 0x8000, 0x30, -2, false);
        }
        else if(t->finished)
        {
            threadJoin(t->thrd, U64_MAX);
            threadFree(t->thrd);
            delete t->status;;
            delete t;
            svcWaitSynchronization(threadLock, U64_MAX);
            threads.erase(threads.begin());
            svcReleaseMutex(threadLock);
        }
    }
}

void ui::threadProcMngr::drawTop()
{
    C2D_DrawRectSolid(0, 0, 0.8f, 400, 240, 0xBB000000);
}

void ui::threadProcMngr::drawBot()
{
    C2D_DrawRectSolid(0, 0, 0.8f, 320, 240, 0xBB000000);

    if(++frameCount % 4 == 0 && ++lgFrame > 7)
        lgFrame = 0;

    if(clrAdd && (clrShft += 6) >= 0x72)
        clrAdd = false;
    else if(!clrAdd && (clrShft -= 3) <= 0x00)
        clrAdd = true;

    uint32_t glyphCol = 0xFF << 24 | (uint8_t)(0xC5 + (clrShft / 2)) << 16 | (uint8_t)(0x88 + clrShft) << 8 | 0x00;
    gfx::drawText(ui::loadGlyphArray[lgFrame], 4, 222, 0.8f, 0.65f, glyphCol);

    svcWaitSynchronization(threadLock, U64_MAX);
    if(!threads[0]->finished)
    {
        if(threads[0]->drawFunc)
            (*(threads[0]->drawFunc))(threads[0]);
        else
        {
            std::string thrdstatus;
            threads[0]->status->getStatus(thrdstatus);
            int txtX = 160 - (gfx::getTextWidth(thrdstatus) / 2);

            gfx::drawText(thrdstatus, txtX, 114, 0.8f, 0.5f, 0xFFFFFFFF);
        }
    }
    svcReleaseMutex(threadLock);
}

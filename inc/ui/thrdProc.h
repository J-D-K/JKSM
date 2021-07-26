#pragma once

#include <vector>

#include "type.h"

namespace ui
{
    class threadProcMngr
    {
        public:
            ~threadProcMngr();

            threadInfo *newThread(ThreadFunc func, void *args, funcPtr _drawFunc);
            void update();
            void drawTop();
            void drawBot();
            bool empty() { return threads.empty(); }

        private:
            std::vector<threadInfo *> threads;
            bool clrAdd = true;
            uint8_t clrShft = 0;
            unsigned frameCount = 0, lgFrame = 0;
            bool threadLock = false;
    };
}

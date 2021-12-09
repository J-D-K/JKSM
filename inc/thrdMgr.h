#pragma once

#include <vector>

#include "type.h"

namespace ui
{
    class threadProcMngr
    {
        public:
            threadProcMngr();
            ~threadProcMngr();

            threadInfo *newThread(ThreadFunc func, void *args, funcPtr _drawFunc);
            void update();
            void drawTop();
            void drawBot();
            bool empty() { return threads.empty(); }

        private:
            int cnt = 0, prio = 0;
            std::vector<threadInfo *> threads;
            bool clrAdd = true;
            uint8_t clrShft = 0;
            unsigned frameCount = 0, lgFrame = 0;
            Handle threadLock;
    };
}
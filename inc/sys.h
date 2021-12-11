#ifndef SYS_H
#define SYS_H

#include "fs.h"

namespace sys
{
    void init();
    void exit();

    extern bool run;
    extern int8_t threadCore;
    extern int threadPrio;
    extern bool isNew3DS;
}

#endif // SYS_H

#ifndef SYS_H
#define SYS_H

#include "fs.h"

namespace sys
{
    void init();
    void exit();

    extern bool run;
    extern uint8_t lang;
}

#endif // SYS_H

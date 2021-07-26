#pragma once

#include <string>
#include <3ds.h>

typedef void (*funcPtr)(void *);

inline void boolLock(bool& lock)
{
    while(lock != false){ }
    lock = true;
}

inline void boolUnlock(bool& lock)
{
    lock = false;
}

typedef struct
{
    bool statusLock = false;
    std::string status;

    void setStatus(const std::string& newStatus)
    {
        boolLock(statusLock);
        status = newStatus;
        boolUnlock(statusLock);
    }

    void getStatus(std::string& out)
    {
        boolLock(statusLock);
        out = status;
        boolUnlock(statusLock);
    }
} threadStatus;

typedef struct
{
    bool running = false, finished = false;
    Thread thrd;
    ThreadFunc thrdFunc;
    void *argPtr = NULL;
    funcPtr drawFunc = NULL;
    threadStatus *status;
} threadInfo;

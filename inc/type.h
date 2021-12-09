#pragma once

#include <string>
#include <3ds.h>

typedef void (*funcPtr)(void *);

class threadStatus
{
    public:
        threadStatus(){ svcCreateMutex(&statusLock, false); }
        ~threadStatus(){ svcCloseHandle(statusLock); }

    void setStatus(const std::string& newStatus)
    {
        svcWaitSynchronization(statusLock, U64_MAX);
        status = newStatus;
        svcReleaseMutex(statusLock);
    }

    void getStatus(std::string& out)
    {
        svcWaitSynchronization(statusLock, U64_MAX);
        out = status;
        svcReleaseMutex(statusLock);
    }

    private:
        Handle statusLock;
        std::string status;
};

class threadInfo
{
    public:
        threadInfo() { svcCreateMutex(&threadLock, false); }
        ~threadInfo() { svcCloseHandle(threadLock); }

        void lock() { svcWaitSynchronization(threadLock, U64_MAX); }
        void unlock() { svcReleaseMutex(threadLock); }

        bool running = false, finished = false;
        Thread thrd;
        ThreadFunc thrdFunc;
        void *argPtr = NULL;
        funcPtr drawFunc = NULL;
        threadStatus *status;
        int thrdID = 0;
    
    private:
        Handle threadLock;
};
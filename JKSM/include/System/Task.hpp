#pragma once
#include "logging/logger.hpp"

#include <cstdarg>
#include <mutex>
#include <string>
#include <thread>

namespace System
{
    /*
        This is how JKSM runs its threaded tasks.
        All functions intended to be used as tasks must follow this signature:
        void FunctionName(System::Task *Task, Arguments...);
    */
    class Task
    {
        public:
            template <typename... Args>
            Task(void (*Function)(System::Task *, Args...), Args... Arguments)
            {
                m_Thread = std::thread(Function, this, std::forward<Args>(Arguments)...);
            }
            // This is so derived classes can pass themselves.
            template <typename TaskType, typename... Args>
            Task(void (*Function)(TaskType *, Args...), TaskType *Task, Args &&...Arguments)
            {
                m_Thread = std::thread(Function, Task, std::forward<Args>(Arguments)...);
            }

            virtual ~Task() { m_Thread.join(); }

            // Returns whether thread has signaled its finished.
            bool IsFinished()
            {
                std::scoped_lock<std::mutex> ThreadLock(m_ThreadLock);
                return m_Finished;
            }

            // Allows thread to signal finishing its task.
            void Finish()
            {
                std::scoped_lock<std::mutex> ThreadLock(m_ThreadLock);
                m_Finished = true;
            }

            // Returns status string.
            std::string GetStatus()
            {
                std::scoped_lock<std::mutex> ThreadLock(m_ThreadLock);
                return m_ThreadStatus;
            }

            // Allows thread to set status string.
            void SetStatus(const char *Format, ...)
            {
                char VaBuffer[0x1000];
                std::va_list VaList;
                va_start(VaList, Format);
                vsnprintf(VaBuffer, 0x1000, Format, VaList);
                va_end(VaList);

                std::scoped_lock<std::mutex> ThreadLock(m_ThreadLock);
                m_ThreadStatus = VaBuffer;
            }

        private:
            // Thread.
            std::thread m_Thread;
            // Mutex
            std::mutex m_ThreadLock;
            // Whether task is still running.
            bool m_Finished = false;
            // String that shows thread's status.
            std::string m_ThreadStatus;
    };
}; // namespace System

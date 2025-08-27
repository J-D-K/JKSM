#include "Logger.hpp"

#include "fslib.hpp"

#include <cstdarg>
#include <mutex>

namespace
{
    // This is the size of the buffer for va args
    static constexpr size_t VA_BUFFER_SIZE = 0x1000;
    // This is the path the log uses.
    const std::u16string_view LOG_FILE_PATH = u"sdmc:/JKSM/JKSM.log";
    // This protects log from being corrupted from threading.
    std::mutex s_LogLock;
} // namespace

void Logger::Initialize(void)
{
    // This will just create and empty log on boot.
    fslib::File LogFile(LOG_FILE_PATH, FS_OPEN_CREATE | FS_OPEN_WRITE);
}

void Logger::Log(const char *Format, ...)
{
    char VaBuffer[VA_BUFFER_SIZE];

    std::va_list VaList;
    va_start(VaList, Format);
    vsnprintf(VaBuffer, VA_BUFFER_SIZE, Format, VaList);
    va_end(VaList);

    std::scoped_lock LogLock(s_LogLock);
    fslib::File LogFile(LOG_FILE_PATH, FS_OPEN_WRITE | FS_OPEN_APPEND);
    LogFile << VaBuffer << "\n";
    // This can be error checked but eh.
    LogFile.flush();
}

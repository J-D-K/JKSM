#include "Logger.hpp"
#include "FsLib.hpp"
#include <cstdarg>

namespace
{
    // This is the size of the buffer for va args
    static constexpr size_t VA_BUFFER_SIZE = 0x1000;
    // This is the path the log uses.
    const std::u16string_view LOG_FILE_PATH = u"sdmc:/JKSM/JKSM.log";
} // namespace

void Logger::Initialize(void)
{
    // This will just create and empty log on boot.
    FsLib::OutputFile LogFile(LOG_FILE_PATH, false);
}

void Logger::Log(const char *Format, ...)
{
    char VaBuffer[VA_BUFFER_SIZE];

    std::va_list VaList;
    va_start(VaList, Format);
    vsnprintf(VaBuffer, VA_BUFFER_SIZE, Format, VaList);
    va_end(VaList);

    FsLib::OutputFile LogFile(LOG_FILE_PATH, true);
    LogFile << VaBuffer << "\n";
    // This can be error checked but eh.
    LogFile.Flush();
}

#include "logging/logger.hpp"

#include "fslib.hpp"

#include <array>
#include <cstdarg>
#include <mutex>

namespace
{
    // This is the size of the buffer for va args
    constexpr size_t VA_BUFFER_SIZE = 0x1000;

    constexpr uint64_t SIZE_LOG_MAX = 0x10000;

    // This is the path the log uses.
    const std::u16string_view LOG_FILE_PATH = u"sdmc:/JKSM/JKSM.log";

    // This protects log from being corrupted from threading.
    std::mutex s_logLock{};
} // namespace

void logger::initialize()
{
    const fslib::Path logPath{LOG_FILE_PATH};

    const bool logExists = fslib::file_exists(LOG_FILE_PATH);
    if (logExists) { return; }

    fslib::create_file(logPath);
}

void logger::log(const char *format, ...)
{
    std::array<char, VA_BUFFER_SIZE> vaBuffer = {0};

    std::va_list vaList;
    va_start(vaList, format);
    std::vsnprintf(vaBuffer.data(), VA_BUFFER_SIZE, format, vaList);
    va_end(vaList);

    const fslib::Path logPath{LOG_FILE_PATH};
    std::lock_guard logGuard{s_logLock};

    uint64_t logSize{};
    fslib::get_file_size(logPath, logSize);
    if (logSize >= SIZE_LOG_MAX)
    {
        fslib::delete_file(logPath);
        fslib::create_file(logPath);
    }

    fslib::File logFile{logPath, FS_OPEN_WRITE | FS_OPEN_APPEND};
    if (!logFile.is_open()) { return; }
    logFile << vaBuffer.data() << "\n";
    logFile.flush();
}

#pragma once

namespace logger
{
    /// @brief Checks to see if the log exists. If not, creates it.
    void initialize();

    /// @brief Logs a formatted line to the log file.
    void log(const char *format, ...);
}

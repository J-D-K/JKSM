#pragma once
#include <cstddef>
#include <string>

namespace StringUtil
{
    typedef enum
    {
        DATE_FMT_YMD,
        DATE_FMT_YDM
    } DateFormats;

    // Removes all forbidden path characters from StringIn and writes the output to StringOut
    void SanitizeStringForPath(const char16_t *StringIn, char16_t *StringOut, size_t StringLength);
    // Returns formatted string
    std::string GetFormattedString(const char *Format, ...);
    // Gets a string with the current date and time for backup naming.
    void GetDateTimeString(char *StringOut, size_t StringLength, StringUtil::DateFormats DateFormat);
    void GetDateTimeString(char16_t *StringOut, size_t StringLength, StringUtil::DateFormats DateFormat);
    // Converts and writes a UTF-16 string to StringOut
    void ToUTF8(const char16_t *String, char *StringOut, size_t StringOutSize);
    // Converts and writes a UTF-8 string to StringOut
    void ToUTF16(const char *String, char16_t *StringOut, size_t StringOutSize);
} // namespace StringUtil

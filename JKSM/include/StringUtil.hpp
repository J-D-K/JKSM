#pragma once
#include <cstddef>
#include <string>

namespace StringUtil
{
    // Removes all forbidden path characters from StringIn and writes the output to StringOut
    void SanitizeStringForPath(const char16_t *StringIn, char16_t *StringOut, size_t StringLength);
    // Returns formatted string
    std::string GetFormattedString(const char *Format, ...);
    // Converts and writes a UTF-16 string to StringOut
    void ToUTF8(const char16_t *String, char *StringOut, size_t StringOutSize);
    // Converts and writes a UTF-8 string to StringOut
    void ToUTF16(const char *String, char16_t *StringOut, size_t StringOutSize);
} // namespace StringUtil

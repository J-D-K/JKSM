#pragma once
#include <cstddef>
#include <string>

namespace StringUtil
{
    // Removes all forbidden path characters from StringIn and writes the output to StringOut
    void SanitizeStringForPath(const char16_t *StringIn, char16_t *StringOut, size_t StringLength);
    // Returns formatted string
    std::string GetFormattedString(const char *Format, ...);
    // Returns the UTF-8 conversion of String.
    std::string ToUTF8(const char16_t *String);
} // namespace StringUtil

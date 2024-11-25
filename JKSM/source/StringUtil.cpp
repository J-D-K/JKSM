#include "StringUtil.hpp"
#include <3ds.h>
#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstring>
#include <string>

namespace
{
    // These are characters that can't be in paths.
    constexpr std::array<char16_t, 11> s_ForbiddenChars = {u'.', u',', u'/', u'\\', u'<', u'>', u':', u'"', u'|', u'?', u'*'};
    // This is the size for VA
    constexpr size_t VA_BUFFER_SIZE = 0x1000;
} // namespace

void StringUtil::SanitizeStringForPath(const char16_t *StringIn, char16_t *StringOut, size_t StringLength)
{
    std::memcpy(StringOut, StringIn, StringLength * sizeof(char16_t));

    while (*StringOut)
    {
        if (std::find(s_ForbiddenChars.begin(), s_ForbiddenChars.end(), *StringOut))
        {
            *StringOut = u' ';
        }
        ++StringOut;
    }

    // Now we need to loop backwards through it and make sure there aren't any trailing spaces and periods.
    size_t SanitizedLength = std::char_traits<char16_t>::length(StringOut);
    while (StringOut[SanitizedLength - 1] == u'.' || StringOut[SanitizedLength - 1] == u' ')
    {
        StringOut[SanitizedLength - 1] = 0x00;
    }
}

std::string StringUtil::GetFormattedString(const char *Format, ...)
{
    char VaBuffer[VA_BUFFER_SIZE] = {0};

    std::va_list VaList;
    va_start(VaList, Format);
    vsnprintf(VaBuffer, VA_BUFFER_SIZE, Format, VaList);
    va_end(VaList);

    return std::string(VaBuffer);
}

void StringUtil::ToUTF8(const char16_t *String, char *StringOut, size_t StringOutSize)
{
    utf16_to_utf8(reinterpret_cast<uint8_t *>(StringOut), reinterpret_cast<const uint16_t *>(String), StringOutSize);
}

void StringUtil::ToUTF16(const char *String, char16_t *StringOut, size_t StringOutSize)
{
    utf8_to_utf16(reinterpret_cast<uint16_t *>(StringOut), reinterpret_cast<const uint8_t *>(String), StringOutSize);
}

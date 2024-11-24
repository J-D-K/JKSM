#include "StringUtil.hpp"
#include <3ds.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <string>

namespace
{
    constexpr std::array<char16_t, 11> s_ForbiddenChars = {u'.', u',', u'/', u'\\', u'<', u'>', u':', u'"', u'|', u'?', u'*'};
}

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

std::string StringUtil::ToUTF8(const char16_t *String)
{
    size_t StringLength = std::char_traits<char16_t>::length(String) + 1;
    char StringBuffer[StringLength] = {0};
    utf16_to_utf8(reinterpret_cast<uint8_t *>(StringBuffer), reinterpret_cast<const uint16_t *>(String), StringLength);
    return std::string(StringBuffer);
}

#pragma once
#include <cstddef>

namespace StringUtil
{
    void SanitizeStringForPath(const char16_t *StringIn, char16_t *StringOut, size_t StringLength);
}

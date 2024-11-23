#pragma once
#include <cstdint>

namespace SDL
{
    union Color
    {
            uint32_t RAW;
            uint8_t RGBA[4];
    };
} // namespace SDL

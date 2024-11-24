#pragma once
#include <cstdint>

namespace SDL
{
    union Color
    {
            uint32_t RAW;
            uint8_t RGBA[4];
    };

    namespace Colors
    {
        static constexpr SDL::Color White = {0xFFFFFFFF};
        static constexpr SDL::Color BarColor = {0x4D4D4DFF};
    } // namespace Colors
} // namespace SDL

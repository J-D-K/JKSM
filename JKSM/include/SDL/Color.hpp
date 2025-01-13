#pragma once
#include <cstdint>

namespace SDL
{
    // This is what I use for colors.
    union Color
    {
            uint32_t RAW;
            uint8_t RGBA[4];
    };
    // This makes pulling individual colors from RGBA easier to read. I don't know what or care about the enum name.
    typedef enum
    {
        Alpha,
        Blue,
        Green,
        Red
    } SubColor;
    // These are the colors I actually use frequently in JKSM.
    namespace Colors
    {
        static constexpr SDL::Color White = {0xFFFFFFFF};
        static constexpr SDL::Color Black = {0x000000FF};
        static constexpr SDL::Color BarColor = {0x4D4D4DFF};
        static constexpr SDL::Color Green = {0x00DD00FF};
        static constexpr SDL::Color DialogBox = {0x5D5D5DFF};
    } // namespace Colors
} // namespace SDL

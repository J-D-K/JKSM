#ifndef GFX_H
#define GFX_H

#include <citro2d.h>
#include <string>

namespace gfx
{
    void init();
    void exit();

    void frameBegin();
    void frameEnd();
    void frameStartTop();
    void frameStartBot();

    void drawText(const std::string& str, const int& x, const int& y, const uint32_t& clr);
    void drawU16Text(const std::u16string& str, const int& x, const int& y, const uint32_t& clr);
    void drawU32Text(const std::u32string& str, const int& x, const int& y, const uint32_t& clr);
    size_t getTextWidth(const std::string& str);
}

#endif // GFX_H

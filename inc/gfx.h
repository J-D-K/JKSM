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
    void drawTextWrap(const std::string& str, const int& x, int y, const int& maxWidth, const uint32_t& clr);
    void drawU16Text(const std::u16string& str, const int& x, const int& y, const uint32_t& clr);
    size_t getTextWidth(const std::string& str);

    void drawBoundingBox(const int& x, const int& y, const int& w, const int& h, const uint32_t& clr, bool light);
}

#endif // GFX_H

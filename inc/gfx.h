#ifndef GFX_H
#define GFX_H

#include <citro2d.h>
#include <string>
#include "type.h"

#define GFX_DEPTH_DEFAULT 0.5f

namespace gfx
{
    void init();
    void exit();

    extern C3D_RenderTarget *top, *bot;
    extern Tex3DS_SubTexture iconSubTex;

    inline void frameBegin()
    {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, 0xFF2D2D2D);
        C2D_TargetClear(bot, 0xFF2D2D2D);
    }

    inline void frameEnd()
    {
        C3D_FrameEnd(0);
    }

    inline void frameStartTop()
    {
        C2D_SceneBegin(top);
    }

    inline void frameStartBot()
    {
        C2D_SceneBegin(bot);
    }

    void drawText(const std::string& str, const int& x, const int& y, const float& depth, const float& txtScale, const uint32_t& clr);
    void drawTextWrap(const std::string& str, const int& x, int y, const float& depth, const float& txtScale, const int& maxWidth, const uint32_t& clr);
    void drawU16Text(const std::u16string& str, const int& x, const int& y, const float& depth, const uint32_t& clr);
    size_t getTextWidth(const std::string& str);

    void drawBoundingBox(const int& x, const int& y, const int& w, const int& h, const float& depth, const uint32_t& clr, bool light);
}

#endif // GFX_H

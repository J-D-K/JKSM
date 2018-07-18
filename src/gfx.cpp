#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>

#include <string>
#include <cstring>
#include <fstream>

#include "gfx.h"
#include "fs.h"
#include "util.h"

static C3D_RenderTarget *top, *bot;

namespace gfx
{
    void init()
    {
        gfxInitDefault();
        C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
        C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
        C2D_Prepare();

        top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
        bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    }

    void exit()
    {
        C2D_Fini();
        C3D_Fini();
        gfxExit();
        romfsExit();
    }

    void frameBegin()
    {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, 0xFF2D2D2D);
        C2D_TargetClear(bot, 0xFF2D2D2D);
    }

    void frameEnd()
    {
        C3D_FrameEnd(0);
    }

    void frameStartTop()
    {
        C2D_SceneBegin(top);
    }

    void frameStartBot()
    {
        C2D_SceneBegin(bot);
    }

    void drawText(const std::string& str, const int& x, const int& y, const uint32_t& clr)
    {
        C2D_Text tmpTxt;
        C2D_TextBuf tmpBuf = C2D_TextBufNew(1024);

        C2D_TextParse(&tmpTxt, tmpBuf, str.c_str());
        C2D_TextOptimize(&tmpTxt);
        C2D_DrawText(&tmpTxt, C2D_WithColor, (float)x, (float) y, 0.5f, 0.5f, 0.5f, clr);
        C2D_TextBufDelete(tmpBuf);
    }

    void drawU16Text(const std::u16string& str, const int& x, const int& y, const uint32_t& clr)
    {
        C2D_Text tmpTxt;
        C2D_TextBuf tmpBuf = C2D_TextBufNew(1024);

        std::string tmp = util::toUtf8(str);

        C2D_TextParse(&tmpTxt, tmpBuf, tmp.c_str());
        C2D_TextOptimize(&tmpTxt);
        C2D_DrawText(&tmpTxt, C2D_WithColor, (float)x, (float)y, 0.5f, 0.5f, 0.5f, clr);
        C2D_TextBufDelete(tmpBuf);
    }

    void drawU32Text(const std::u32string& str, const int& x, const int& y, const uint32_t& clr)
    {
        C2D_Text tmpTxt;
        C2D_TextBuf tmpBuf = C2D_TextBufNew(1024);

        std::string tmp = util::toUtf8(str);

        C2D_TextParse(&tmpTxt, tmpBuf, tmp.c_str());
        C2D_TextOptimize(&tmpTxt);
        C2D_DrawText(&tmpTxt, C2D_WithColor, (float)x, (float) y, 0.5f, 0.5f, 0.5f, clr);
        C2D_TextBufDelete(tmpBuf);
    }

    size_t getTextWidth(const std::string& str)
    {
        float ret = 0;
        C2D_Text tmpTxt;
        C2D_TextBuf tmpBuf = C2D_TextBufNew(1024);

        C2D_TextParse(&tmpTxt, tmpBuf, str.c_str());
        C2D_TextOptimize(&tmpTxt);

        C2D_TextGetDimensions(&tmpTxt, 0.5f, 0.5f, &ret, NULL);
        C2D_TextBufDelete(tmpBuf);

        return static_cast<size_t>(ret);
    }
}

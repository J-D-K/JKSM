#pragma once
#include "SDL/Color.hpp"
#include "SDL/ResourceManager.hpp"
#include <SDL/SDL.h>
#include <ft2build.h>
#include <map>
#include <memory>
#include <string_view>
#include FT_FREETYPE_H

namespace SDL
{
    // This is for caching glyph surfaces and data to blit them.
    typedef struct
    {
            int16_t AdvanceX, Top, Left;
            SDL::SharedSurface GlyphSurface;
    } FontGlyph;

    // I don't really think Freetype needs a completely separate header. This is the only thing that relies on it.
    namespace FreeType
    {
        bool Initialize(void);
        void Exit(void);
    } // namespace FreeType

    class Font
    {
        public:
            Font(void) = default;
            // Loads font zlib compressed ttf font into RAM and inits Freetype. TextColor is the color to use when creating glyph surfaces.
            Font(std::string_view FontPath, SDL::Color TextColor);
            // Cleans up free type.
            ~Font();
            /*
                Blits text at X, Y at FontSize in pixels. WrapWidth is the maximum text width to reach before wrapping to a new line. Font.NoWrap
                or -1 can be passed if no wrapping is needed.
            */
            void BlitTextAt(SDL_Surface *Target, int X, int Y, int FontSize, int WrapWidth, const char *Format, ...);
            // Returns the width of the text at FontSize in pixels.
            size_t GetTextWidth(int FontSize, const char *Text);

            // This is so it's easier to read what's going on with blitting.
            static constexpr int NO_TEXT_WRAP = -1;

        private:
            // FreeType face
            FT_Face m_FTFace = nullptr;
            // This saves the current font size to prevent multiple calls to FT_Set_Pixel_Sizes. My font class can handle any size and isn't static like others... lol
            int m_FontSize = 0;
            // Base color to use to construct glyphs.
            SDL::Color m_TextColor;
            // Buffer to hold font in RAM because reading and rendering from SD, especially on 3DS, is ungodly slow.
            std::unique_ptr<FT_Byte[]> m_FontBuffer = nullptr;
            // Map to hold cached characters by codepoint and size so we only need to render with FreeType once.
            std::map<std::pair<uint32_t, int>, SDL::FontGlyph> m_GlyphCacheMap;
            // Resizes font to FontSize in pixels
            void ResizeFont(int FontSize);
            // Searches for glyph in map, if it's not found, uses Freetype to render it. If neither are possible, returns nullptr.
            FontGlyph *SearchLoadGlyph(uint32_t Codepoint, int FontSize, FT_Int32 FreetypeLoadFlags);
    };
} // namespace SDL

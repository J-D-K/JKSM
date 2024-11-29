#include "SDL/Font.hpp"
#include "FsLib.hpp"
#include "Logger.hpp"
#include "SDL/SDL.hpp"
#include <3ds.h>
#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <zlib.h>

namespace
{
    // Freetype library.
    FT_Library s_FTLib = nullptr;
    // This is the size of the buffer used for va args.
    constexpr size_t VA_BUFFER_SIZE = 0x1000;
    // This is the length of the buffer used for text wrapping.
    constexpr size_t WORD_BUFFER_SIZE = 0x100;
    // This is the array of characters lines can be wrapped at.
    constexpr std::array<uint32_t, 7> s_Breakpoints = {L' ', L'　', L'/', L'_', L'-', L'。', L'、'};
} // namespace

// These are helper functions that don't really belong in the class.
static size_t FindNextBreakpoint(const char *String)
{
    uint32_t Codepoint = 0x00;
    size_t StringLength = std::char_traits<char>::length(String);
    for (size_t i = 0; i < StringLength;)
    {
        ssize_t UnitCount = decode_utf8(&Codepoint, reinterpret_cast<const uint8_t *>(&String[i]));

        // This might not actually be right, but it works for now.
        i += UnitCount;
        // If decode_utf8 failed or Codepoint is a breakpoint we can use.
        if (UnitCount <= 0 || std::find(s_Breakpoints.begin(), s_Breakpoints.end(), Codepoint) != s_Breakpoints.end())
        {
            return i;
        }
    }
    return StringLength;
}

bool SDL::FreeType::Initialize(void)
{
    FT_Error FTError = FT_Init_FreeType(&s_FTLib);
    if (FTError != 0)
    {
        Logger::Log("Error intializing FreeType: %i.", FTError);
        return false;
    }
    return true;
}

void SDL::FreeType::Exit(void)
{
    if (s_FTLib)
    {
        FT_Done_FreeType(s_FTLib);
    }
}

SDL::Font::Font(std::string_view FontPath, SDL::Color TextColor) : m_TextColor({TextColor.RAW})
{
    // Unfortunately, I'm not sure FsLib is ever going to get RomFS support. Still need stdio or fstream for this...
    std::FILE *FontFile = std::fopen(FontPath.data(), "rb");
    if (!FontFile)
    {
        Logger::Log("Error opening font file for reading.");
        return;
    }

    // These are needed for decompressing the font.
    uLongf UncompressedFontsize = 0;
    uLongf CompressedFontSize = 0;
    fread(&UncompressedFontsize, sizeof(uLongf), 1, FontFile);
    fread(&CompressedFontSize, sizeof(uLongf), 1, FontFile);

    {
        // Allocate buffer to read compressed data.
        std::unique_ptr<FT_Byte[]> CompressedBuffer(new FT_Byte[CompressedFontSize]);
        if (!CompressedBuffer)
        {
            Logger::Log("Error allocating CompressedBuffer");
            return;
        }
        // Read it.
        size_t ReadSize = std::fread(CompressedBuffer.get(), 1, CompressedFontSize, FontFile);
        if (ReadSize != CompressedFontSize)
        {
            Logger::Log("Error reading full compresed size font.");
            return;
        }

        // This is the buffer the font actually keeps.
        m_FontBuffer = std::make_unique<FT_Byte[]>(UncompressedFontsize);
        int ZlibError = uncompress(m_FontBuffer.get(),
                                   reinterpret_cast<uLongf *>(&UncompressedFontsize),
                                   CompressedBuffer.get(),
                                   static_cast<uLongf>(CompressedFontSize));
        if (ZlibError != Z_OK)
        {
            Logger::Log("Error decompressing font: %i.", ZlibError);
            return;
        }
    }

    FT_Error FTError = FT_New_Memory_Face(s_FTLib, m_FontBuffer.get(), UncompressedFontsize, 0, &m_FTFace);
    if (FTError != 0)
    {
        Logger::Log("Error creating new FreeType face: %i.", FTError);
        return;
    }
}

SDL::Font::~Font()
{
    if (m_FTFace)
    {
        FT_Done_Face(m_FTFace);
    }
}

void SDL::Font::BlitTextAt(SDL_Surface *Target, int X, int Y, int FontSize, int WrapWidth, const char *Format, ...)
{
    if (!m_FTFace)
    {
        return;
    }

    // Va arg the text passed.
    char VaBuffer[VA_BUFFER_SIZE] = {0};

    std::va_list VaList;
    va_start(VaList, Format);
    vsnprintf(VaBuffer, VA_BUFFER_SIZE, Format, VaList);
    va_end(VaList);

    // Get string's length for loop.
    size_t StringLength = std::char_traits<char>::length(VaBuffer);
    // We need to preserve the original X position passed.
    int WorkingX = X;
    // Current character codepoint.
    uint32_t Codepoint = 0;

    Font::ResizeFont(FontSize);

    for (size_t i = 0; i < StringLength;)
    {
        // This buffer is used for chopping the string into words and calculating width to break lines at.
        std::array<char, WORD_BUFFER_SIZE> WordBuffer = {0};

        // Get the length of the current word and memcpy it to WordBuffer
        size_t CurrentWordLength = FindNextBreakpoint(&VaBuffer[i]);
        std::memcpy(WordBuffer.data(), &VaBuffer[i], CurrentWordLength);

        // Wrap line if needed.
        if (WrapWidth != SDL::Font::NO_TEXT_WRAP)
        {
            size_t WordWidth = Font::GetTextWidth(FontSize, WordBuffer.data());
            if (WorkingX + WordWidth >= static_cast<size_t>(X + WrapWidth))
            {
                WorkingX = X;
                Y += FontSize + (FontSize / 3);
            }
        }

        // Loop through word buffer and blit it to screen.
        for (size_t j = 0; j < CurrentWordLength;)
        {
            ssize_t UnitCount = decode_utf8(&Codepoint, reinterpret_cast<const uint8_t *>(&WordBuffer[j]));
            if (UnitCount <= 0)
            {
                break;
            }

            j += UnitCount;
            // Process newline chars.
            if (Codepoint == L'\n')
            {
                WorkingX = X;
                Y += FontSize + (FontSize / 3);
                continue;
            }

            // Finally pull or load glyph from map.
            FontGlyph *CurrentGlyph = Font::SearchLoadGlyph(Codepoint, FontSize, FT_LOAD_RENDER);
            if (CurrentGlyph && Codepoint != L' ')
            {
                CurrentGlyph->GlyphSurface->BlitAt(Target, WorkingX + CurrentGlyph->Left, Y + (FontSize - CurrentGlyph->Top));
                WorkingX += CurrentGlyph->AdvanceX;
            }
            else if (CurrentGlyph) // Space needs special handling because its surface is NULL
            {
                WorkingX += CurrentGlyph->AdvanceX;
            }
        }
        i += CurrentWordLength;
    }
}

size_t SDL::Font::GetTextWidth(int FontSize, const char *Text)
{
    uint32_t Codepoint = 0;
    size_t TextWidth = 0;
    size_t StringLength = std::char_traits<char>::length(Text);

    ResizeFont(FontSize);

    for (size_t i = 0; i < StringLength;)
    {
        ssize_t UnitCount = decode_utf8(&Codepoint, reinterpret_cast<const uint8_t *>(&Text[i]));
        if (UnitCount <= 0)
        {
            return TextWidth;
        }

        i += UnitCount;
        if (Codepoint == L'\n')
        {
            continue;
        }

        FontGlyph *CurrentGlyph = Font::SearchLoadGlyph(Codepoint, FontSize, FT_LOAD_RENDER);
        if (CurrentGlyph)
        {
            TextWidth += CurrentGlyph->AdvanceX;
        }
    }
    return TextWidth;
}

void SDL::Font::ResizeFont(int FontSize)
{
    if (m_FontSize == FontSize)
    {
        return;
    }

    m_FontSize = FontSize;
    FT_Error FTError = FT_Set_Pixel_Sizes(m_FTFace, 0, static_cast<FT_UInt>(m_FontSize));
    if (FTError != 0)
    {
        Logger::Log("Error setting font size in pixels: %i.", FTError);
    }
}

SDL::FontGlyph *SDL::Font::SearchLoadGlyph(uint32_t Codepoint, int FontSize, FT_Int32 FreeTypeLoadFlags)
{
    if (m_GlyphCacheMap.find(std::make_pair(Codepoint, FontSize)) != m_GlyphCacheMap.end())
    {
        return &m_GlyphCacheMap.at(std::make_pair(Codepoint, FontSize));
    }

    FT_UInt CodepointIndex = FT_Get_Char_Index(m_FTFace, Codepoint);
    FT_Error FTError = FT_Load_Glyph(m_FTFace, CodepointIndex, FreeTypeLoadFlags);
    if (CodepointIndex == 0 || FTError != 0 || m_FTFace->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
    {
        Logger::Log("Glyph could not be loaded.");
        return nullptr;
    }
    // Pointer to bitmap in FTFace
    FT_Bitmap GlyphBitmap = m_FTFace->glyph->bitmap;
    // Allocate surface for glyph.

    // Glyph needs a name for map.
    std::string GlyphName = std::to_string(Codepoint) + " - " + std::to_string(FontSize);
    SDL::SharedSurface GlyphSurface = SDL::SurfaceManager::CreateLoadResource(GlyphName, GlyphBitmap.width, GlyphBitmap.rows);
    if (!GlyphSurface)
    {
        return nullptr;
    }

    // This is for iterating through to render or construct the glyph.
    size_t BitmapSize = GlyphBitmap.width * GlyphBitmap.rows;
    unsigned char *BitmapBuffer = GlyphBitmap.buffer;
    uint32_t *SurfacePixels = reinterpret_cast<uint32_t *>(GlyphSurface->Get()->pixels);

    // Loop through and fill out the pixels in surface.
    for (size_t i = 0; i < BitmapSize; i++)
    {
        SurfacePixels[i] = ((m_TextColor.RAW & 0xFFFFFF00) | BitmapBuffer[i]);
    }

    m_GlyphCacheMap[std::make_pair(Codepoint, FontSize)] = {.AdvanceX = static_cast<int16_t>(m_FTFace->glyph->advance.x >> 6),
                                                            .Top = static_cast<int16_t>(m_FTFace->glyph->bitmap_top),
                                                            .Left = static_cast<int16_t>(m_FTFace->glyph->bitmap_left),
                                                            .GlyphSurface = GlyphSurface};

    return &m_GlyphCacheMap.at(std::make_pair(Codepoint, FontSize));
}

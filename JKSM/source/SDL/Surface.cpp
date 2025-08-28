#include "SDL/Surface.hpp"

#include "SDL/SDL.hpp"
#include "logging/logger.hpp"

#include <cstdio>
#include <png.h>

namespace
{
    constexpr uint32_t RED_MASK   = 0xFF000000;
    constexpr uint32_t GREEN_MASK = 0x00FF0000;
    constexpr uint32_t BLUE_MASK  = 0x0000FF00;
    constexpr uint32_t ALPHA_MASK = 0x000000FF;
} // namespace

static void PNGCleanup(std::FILE *PNGFile, png_structpp ReadStruct, png_infopp InfoStruct)
{
    // I'm hoping this checks for NULLs...
    if (ReadStruct) { png_destroy_read_struct(ReadStruct, InfoStruct, NULL); }

    if (PNGFile) { std::fclose(PNGFile); }
}

SDL::Surface::Surface(int Width, int Height, bool AlphaBlended)
{
    m_Surface = SDL_CreateRGBSurface(0, Width, Height, 32, RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
    if (!m_Surface) { logger::log("Error creating %ix%i surface: %s.", SDL_GetError()); }

    if (!AlphaBlended) { Surface::DisableAlphaBlending(); }
}

SDL::Surface::Surface(std::string_view FilePath, bool AlphaBlended)
{
    // Try opening file first to make sure it actually exists.
    std::FILE *PNGFile = std::fopen(FilePath.data(), "rb");
    if (!PNGFile) { return; }

    // Read struct.
    png_structp ReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    // Info struct.
    png_infop InfoStruct = png_create_info_struct(ReadStruct);
    // I'm hoping (and assuming) that failing to allocate ReadStruct will cause InfoStruct to fail to allocate. I don't know if
    // this is true. Potential memory leak?
    if (!ReadStruct)
    {
        PNGCleanup(PNGFile, &ReadStruct, &InfoStruct);
        return;
    }

    // Init IO.
    png_init_io(ReadStruct, PNGFile);

    // Read info from png.
    png_read_info(ReadStruct, InfoStruct);

    // Check if format is correct. We only want RGBA8 PNGs.
    if (png_get_color_type(ReadStruct, InfoStruct) != PNG_COLOR_TYPE_RGBA)
    {
        PNGCleanup(PNGFile, &ReadStruct, &InfoStruct);
        return;
    }

    // Get width and height for new surface.
    int PNGWidth  = png_get_image_width(ReadStruct, InfoStruct);
    int PNGHeight = png_get_image_height(ReadStruct, InfoStruct);

    // Allocate surface
    m_Surface = SDL_CreateRGBSurface(0, PNGWidth, PNGHeight, 32, RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
    if (!m_Surface)
    {
        PNGCleanup(PNGFile, &ReadStruct, &InfoStruct);
        return;
    }

    // Loop and read PNG row by row into surface pixel buffer.
    unsigned char *SurfacePixels = reinterpret_cast<unsigned char *>(m_Surface->pixels);
    for (int i = 0; i < PNGHeight; i++)
    {
        // Read row directly into surface
        png_read_row(ReadStruct, &SurfacePixels[i * (PNGWidth * 4)], NULL);
    }

    // Cleanup
    PNGCleanup(PNGFile, &ReadStruct, &InfoStruct);

    // Endianess of Pixels needs to be corrected.
    uint32_t *PixelPointer = reinterpret_cast<uint32_t *>(m_Surface->pixels);
    for (int i = 0; i < PNGWidth * PNGHeight; i++)
    {
        uint32_t PixelData = *PixelPointer;
        *PixelPointer++    = ((PixelData << 24) & RED_MASK) | ((PixelData << 8) & GREEN_MASK) | ((PixelData >> 8) & BLUE_MASK) |
                          ((PixelData >> 24) & ALPHA_MASK);
    }

    if (!AlphaBlended) { Surface::DisableAlphaBlending(); }
}

SDL::Surface::Surface(SDL_Surface *ExternalSurface, bool AlphaBlended)
{
    m_Surface = ExternalSurface;

    if (!AlphaBlended) { Surface::DisableAlphaBlending(); }
}

SDL::Surface::~Surface() { SDL_FreeSurface(m_Surface); }

SDL_Surface *SDL::Surface::Get() { return m_Surface; }

void SDL::Surface::BlitAt(SDL_Surface *Target, int X, int Y)
{
    if (!m_Surface) { return; }
    SDL_Rect DestinationRect = {.x = static_cast<int16_t>(X),
                                .y = static_cast<int16_t>(Y),
                                .w = static_cast<uint16_t>(m_Surface->w),
                                .h = static_cast<uint16_t>(m_Surface->h)};

    SDL_BlitSurface(m_Surface, NULL, Target, &DestinationRect);
}

void SDL::Surface::BlitPartAt(SDL_Surface *Target, int X, int Y, int SourceX, int SourceY, int SourceWidth, int SourceHeight)
{
    if (!m_Surface) { return; }

    SDL_Rect SourceRect      = {.x = static_cast<int16_t>(SourceX),
                                .y = static_cast<int16_t>(SourceY),
                                .w = static_cast<uint16_t>(SourceWidth),
                                .h = static_cast<uint16_t>(SourceHeight)};
    SDL_Rect DestinationRect = {.x = static_cast<int16_t>(X),
                                .y = static_cast<int16_t>(Y),
                                .w = static_cast<uint16_t>(SourceWidth),
                                .h = static_cast<uint16_t>(SourceHeight)};

    SDL_BlitSurface(m_Surface, &SourceRect, Target, &DestinationRect);
}

void SDL::Surface::ChangePixelsToColor(SDL::Color Color)
{
    uint32_t *PixelData = reinterpret_cast<uint32_t *>(m_Surface->pixels);
    for (int i = 0; i < m_Surface->w * m_Surface->h; i++)
    {
        PixelData[i] = Color.RGBA[SDL::Red] << 24 | Color.RGBA[SDL::Green] << 16 | Color.RGBA[SDL::Blue] << 8 |
                       (PixelData[i] & ALPHA_MASK);
    }
}

void SDL::Surface::DisableAlphaBlending()
{
    if (SDL_SetAlpha(m_Surface, 0, 0xFF) != 0)
    {
        logger::log("Error disabling alpha blending for surface: %s", SDL_GetError());
    }
}

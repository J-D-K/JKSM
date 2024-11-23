#include "SDL/Surface.hpp"
#include "Logger.hpp"
#include "SDL/SDL.hpp"
#include <SDL/SDL_image.h>

namespace
{
    constexpr uint32_t RED_MASK = 0xFF000000;
    constexpr uint32_t GREEN_MASK = 0x00FF0000;
    constexpr uint32_t BLUE_MASK = 0x0000FF00;
    constexpr uint32_t ALPHA_MASK = 0x000000FF;
} // namespace

SDL::Surface::Surface(int Width, int Height, bool AlphaBlended)
{
    m_Surface = SDL_CreateRGBSurface(0, Width, Height, 32, RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
    if (!m_Surface)
    {
        Logger::Log("Error creating %ix%i surface: %s.", SDL_GetError());
    }

    if (!AlphaBlended)
    {
        Surface::DisableAlphaBlending();
    }
}

SDL::Surface::Surface(std::string_view FilePath, bool AlphaBlended)
{
    m_Surface = IMG_Load(FilePath.data());
    if (!m_Surface)
    {
        Logger::Log("Error loading image: %s.", IMG_GetError());
    }

    if (!AlphaBlended)
    {
        Surface::DisableAlphaBlending();
    }
}

SDL::Surface::Surface(SDL_Surface *ExternalSurface, bool AlphaBlended)
{
    m_Surface = ExternalSurface;

    if (!AlphaBlended)
    {
        Surface::DisableAlphaBlending();
    }
}

SDL::Surface::~Surface()
{
    SDL_FreeSurface(m_Surface);
}

SDL_Surface *SDL::Surface::Get(void)
{
    return m_Surface;
}

void SDL::Surface::BlitAt(SDL_Surface *Target, int X, int Y)
{
    if (!m_Surface)
    {
        return;
    }
    SDL_Rect DestinationRect = {.x = static_cast<int16_t>(X),
                                .y = static_cast<int16_t>(Y),
                                .w = static_cast<uint16_t>(m_Surface->w),
                                .h = static_cast<uint16_t>(m_Surface->h)};

    SDL_BlitSurface(m_Surface, NULL, Target, &DestinationRect);
}

void SDL::Surface::BlitPartAt(SDL_Surface *Target, int X, int Y, int SourceX, int SourceY, int SourceWidth, int SourceHeight)
{
    if (!m_Surface)
    {
        return;
    }

    SDL_Rect SourceRect = {.x = static_cast<int16_t>(SourceX),
                           .y = static_cast<int16_t>(SourceY),
                           .w = static_cast<uint16_t>(SourceWidth),
                           .h = static_cast<uint16_t>(SourceHeight)};
    SDL_Rect DestinationRect = {.x = static_cast<int16_t>(X),
                                .y = static_cast<int16_t>(Y),
                                .w = static_cast<uint16_t>(SourceWidth),
                                .h = static_cast<uint16_t>(SourceHeight)};

    SDL_BlitSurface(m_Surface, &SourceRect, Target, &DestinationRect);
}

void SDL::Surface::DisableAlphaBlending(void)
{
    if (SDL_SetAlpha(m_Surface, 0, 0xFF) != 0)
    {
        Logger::Log("Error disabling alpha blending for surface: %s", SDL_GetError());
    }
}

#include "SDL/SDL.hpp"
#include "Logger.hpp"
#include <cstring>

namespace
{
    // This is the framebuffer.
    SDL::SharedSurface s_FrameBuffer = nullptr;
    // These are buffers for each screen to make doing this easier.
    SDL::SharedSurface s_TopScreen = nullptr, s_BottomScreen = nullptr;
    // This is the pointer to the current buffer set.
    SDL_Surface *s_CurrentBuffer = nullptr;
    // These are the names of the surfaces for the map.
    constexpr std::string_view FRAMEBUFFER_NAME = "FRAMEBUFFER";
    constexpr std::string_view TOP_SCREEN_NAME = "TOP_SCREEN_BUFFER";
    constexpr std::string_view BOTTOM_SCREEN_NAME = "BOTTOM_SCREEN_NAME";
    // These are the coordinates for bliting the top and bottom buffers to the framebuffer.
    SDL_Rect s_TopCoords = {.x = 0, .y = 0, .w = 400, .h = 240};
    SDL_Rect s_BottomCoords = {.x = 40, .y = 240, .w = 320, .h = 240};
} // namespace

bool SDL::Initialize(void)
{
    int SDLError = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    if (SDLError)
    {
        Logger::Log("Error initializing SDL: %s", SDL_GetError());
        return false;
    }
    // This is the main framebuffer surface. I don't like needing to offset coordinates to blit to the bottom screen.
    s_FrameBuffer = SDL::SurfaceManager::CreateLoadResource(FRAMEBUFFER_NAME, SDL_SetVideoMode(400, 480, 32, SDL_DUALSCR), false);
    if (!s_FrameBuffer)
    {
        Logger::Log("Error setting video mode: %s", SDL_GetError());
        return false;
    }

    // All of these buffers are set to not use alpha to make blits faster.
    // Top screen buffer
    s_TopScreen = SDL::SurfaceManager::CreateLoadResource(TOP_SCREEN_NAME, 400, 240, false);
    if (!s_TopScreen)
    {
        Logger::Log("Error allocating top screen buffer: %s", SDL_GetError());
        return false;
    }

    // Bottom screen buffer.
    s_BottomScreen = SDL::SurfaceManager::CreateLoadResource(BOTTOM_SCREEN_NAME, 320, 240, false);
    if (!s_BottomScreen)
    {
        Logger::Log("Error allocating bottom screen buffer: %s", SDL_GetError());
        return false;
    }

    // Get rid of the annoying cursor
    SDL_ShowCursor(false);

    return true;
}

void SDL::Exit(void)
{
    SDL_Quit();
}

void SDL::FrameBegin(void)
{
    // Clear both screen buffers to the default Switch UI color.
    std::memset(s_TopScreen->Get()->pixels, 0x2D, 400 * 240 * sizeof(uint32_t));
    std::memset(s_BottomScreen->Get()->pixels, 0x2D, 320 * 240 * sizeof(uint32_t));

    // Set buffer to top
    s_CurrentBuffer = s_TopScreen->Get();
}

void SDL::FrameChangeScreens(void)
{
    s_CurrentBuffer = s_BottomScreen->Get();
}

void SDL::FrameEnd(void)
{
    SDL_BlitSurface(s_TopScreen->Get(), NULL, s_FrameBuffer->Get(), &s_TopCoords);
    SDL_BlitSurface(s_BottomScreen->Get(), NULL, s_FrameBuffer->Get(), &s_BottomCoords);

    SDL_Flip(s_FrameBuffer->Get());
}

SDL_Surface *SDL::GetCurrentBuffer(void)
{
    return s_CurrentBuffer;
}

void SDL::DrawRect(SDL_Surface *Target, int X, int Y, int Width, int Height, SDL::Color Color)
{
    SDL_Rect Coordinates = {.x = static_cast<int16_t>(X),
                            .y = static_cast<int16_t>(Y),
                            .w = static_cast<uint16_t>(Width),
                            .h = static_cast<uint16_t>(Height)};
    SDL_FillRect(Target, &Coordinates, Color.RAW);
}

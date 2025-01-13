#include "UI/Draw.hpp"
#include "Assets.hpp"
#include "SDL/Color.hpp"

namespace
{
    // This is what I use to draw the rounded corners on the dialog box.
    SDL::SharedSurface s_DialogCorners = nullptr;
    // Same as above, but with selection boxes
    SDL::SharedSurface s_BoundingCorners = nullptr;
} // namespace

void UI::DrawDialogBox(SDL_Surface *Target, int X, int Y, int Width, int Height)
{
    if (!s_DialogCorners)
    {
        s_DialogCorners = SDL::SurfaceManager::CreateLoadResource(Asset::Names::DIALOG_BOX, Asset::Paths::DIALOG_BOX_PATH, true);
    }

    // Top
    s_DialogCorners->BlitPartAt(Target, X, Y, 0, 0, 16, 16);
    SDL::DrawRect(Target, X + 16, Y, Width - 32, 16, SDL::Colors::DialogBox);
    s_DialogCorners->BlitPartAt(Target, X + (Width - 16), Y, 16, 0, 16, 16);
    // Middle
    SDL::DrawRect(Target, X, Y + 16, Width, Height - 32, SDL::Colors::DialogBox);
    // Bottom
    s_DialogCorners->BlitPartAt(Target, X, Y + (Height - 16), 0, 16, 16, 16);
    SDL::DrawRect(Target, X + 16, Y + (Height - 16), Width - 32, 16, SDL::Colors::DialogBox);
    s_DialogCorners->BlitPartAt(Target, X + (Width - 16), Y + (Height - 16), 16, 16, 16, 16);
}

void UI::DrawBoundingBox(SDL_Surface *Target, int X, int Y, int Width, int Height, uint8_t ColorMod)
{
    if (!s_BoundingCorners)
    {
        s_BoundingCorners = SDL::SurfaceManager::CreateLoadResource(Asset::Names::BOUNDING_CORNERS, Asset::Paths::BOUNDING_CORNERS_PATH, true);
    }

    //We're going to just assign and edit this.
    SDL::Color BoxColor = {0x0088C5FF};
    BoxColor.RGBA[SDL::Green] = 0x88 + ColorMod;
    BoxColor.RGBA[SDL::Blue] = 0xC5 + (ColorMod / 2);
    // Change bounding for pulse.
    s_BoundingCorners->ChangePixelsToColor(BoxColor);

    // Top
    s_BoundingCorners->BlitPartAt(Target, X, Y, 0, 0, 3, 3);
    SDL::DrawRect(Target, X + 3, Y, Width - 6, 3, BoxColor);
    s_BoundingCorners->BlitPartAt(Target, (X + Width) - 3, Y, 3, 0, 3, 3);

    // Middle
    SDL::DrawRect(Target, X, Y + 3, 3, Height - 6, BoxColor);
    SDL::DrawRect(Target, X + (Width - 3), Y + 3, 3, Height - 6, BoxColor);

    // Bottom
    s_BoundingCorners->BlitPartAt(Target, X, Y + (Height - 3), 0, 3, 3, 3);
    SDL::DrawRect(Target, X + 3, Y + (Height - 3), Width - 6, 3, BoxColor);
    s_BoundingCorners->BlitPartAt(Target, X + (Width - 3), Y + (Height - 3), 3, 3, 3, 3);
}

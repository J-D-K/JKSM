#include "UI/Draw.hpp"
#include "Assets.hpp"

namespace
{
    // This is what I use to draw the rounded corners on the dialog box.
    SDL::SharedSurface s_DialogCorners = nullptr;
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

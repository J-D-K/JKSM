#include "UI/TitleTile.hpp"

UI::TitleTile::TitleTile(bool IsFavorite, SDL::SharedSurface Icon) : m_IsFavorite(IsFavorite), m_Icon(Icon)
{
}

void UI::TitleTile::DrawAt(SDL_Surface *Target, int X, int Y)
{
    m_Icon->BlitAt(Target, X, Y);
}

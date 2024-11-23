#include "UI/TitleView.hpp"
#include "Input.hpp"
#include <cmath>

UI::TitleView::TitleView(Data::SaveDataType SaveType) : m_SaveType(SaveType)
{
    TitleView::Refresh();
}

void UI::TitleView::Update(void)
{
    int TileTotal = m_TitleTiles.size() - 1;

    if (Input::ButtonPressed(KEY_DUP) && (m_Selected -= 7) < 0)
    {
        m_Selected = 0;
    }
    else if (Input::ButtonPressed(KEY_DDOWN) && (m_Selected += 7) > TileTotal)
    {
        m_Selected = TileTotal;
    }
    else if (Input::ButtonPressed(KEY_DLEFT) && --m_Selected < 0)
    {
        m_Selected = 0;
    }
    else if (Input::ButtonPressed(KEY_DRIGHT) && ++m_Selected > TileTotal)
    {
        m_Selected = TileTotal;
    }
}

void UI::TitleView::Draw(SDL_Surface *Target)
{
    if (m_TitleTiles.size() <= 0)
    {
        return;
    }

    // I know some of these are magic numbers and I can't remember how I came up with them. I just remember this being like a balancing act.
    if (m_SelectionY > 176)
    {
        double Add = (176.0f - static_cast<double>(m_SelectionY)) / 3.0f;
        m_Y += std::ceil(Add);
    }
    else if (m_SelectionY < 21)
    {
        double Add = (21.0f - static_cast<double>(m_SelectionY)) / 3.0f;
        m_Y += std::ceil(Add);
    }

    for (size_t TemporaryY = 24, i = 0; i < m_TitleTiles.size(); TemporaryY += 56)
    {
        size_t RowEnd = i + 7;
        for (size_t TemporaryX = 14; i < (RowEnd > m_TitleTiles.size() ? m_TitleTiles.size() : RowEnd); TemporaryX += 54, i++)
        {
            if (i == static_cast<size_t>(m_Selected))
            {
                m_SelectionX = TemporaryX - 4;
                m_SelectionY = TemporaryY - 4;
                SDL::DrawRect(Target, m_SelectionX, m_SelectionY, 56, 56, {0x00FFFFFF});
            }
            m_TitleTiles[i].DrawAt(Target, TemporaryX, TemporaryY);
        }
    }
}

void UI::TitleView::Refresh(void)
{
    // Clear current tiles.
    m_TitleTiles.clear();
    // Get vector of titledata
    Data::GetTitlesWithType(m_SaveType, m_TitleData);

    for (Data::TitleData *CurrentTitle : m_TitleData)
    {
        m_TitleTiles.emplace_back(CurrentTitle->IsFavorite(), CurrentTitle->GetIcon());
    }
}

void UI::TitleView::SetSelected(int Selected)
{
    m_Selected = Selected;
}

int UI::TitleView::GetSelected(void) const
{
    return m_Selected;
}

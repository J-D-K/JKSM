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

    // I know some of these are magic numbers and I can't remember how I came up with them. I just remember this being like a balancing act.
    if (m_SelectionY > 160)
    {
        double Add = (160.0f - static_cast<double>(m_SelectionY)) / 2.0f;
        m_Y += std::ceil(Add);
    }
    else if (m_SelectionY < 18)
    {
        double Add = (18.0f - static_cast<double>(m_SelectionY)) / 2.0f;
        m_Y += std::ceil(Add);
    }
}

void UI::TitleView::Draw(SDL_Surface *Target)
{
    if (m_TitleTiles.size() <= 0)
    {
        return;
    }

    for (int TemporaryY = m_Y, i = 0; i < static_cast<int>(m_TitleTiles.size()); TemporaryY += 56)
    {
        size_t RowEnd = i + 7;
        for (int TemporaryX = m_X; i < static_cast<int>(RowEnd > m_TitleTiles.size() ? m_TitleTiles.size() : RowEnd); TemporaryX += 54, i++)
        {
            if (i == m_Selected)
            {
                m_SelectionX = TemporaryX - 4;
                m_SelectionY = TemporaryY - 4;
                SDL::DrawRect(Target, m_SelectionX, m_SelectionY, 56, 56, {0x00FFFFFF});
            }

            // Don't bother wasting time processing or bliting stuff we can't see anyway.
            if (TemporaryY < -32 || TemporaryY > 224)
            {
                continue;
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

Data::TitleData *UI::TitleView::GetSelectedTitleData(void)
{
    return m_TitleData.at(m_Selected);
}

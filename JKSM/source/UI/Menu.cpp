#include "UI/Menu.hpp"
#include "Assets.hpp"
#include "Input.hpp"
#include <3ds.h>

UI::Menu::Menu(int X, int Y, int Width, int MaxDrawLength)
    : m_X(X), m_Y(Y), m_Width(Width), m_MaximumDrawLength(MaxDrawLength - 1),
      m_Noto(SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White))
{
}

UI::Menu::Menu(int X, int Y, int Width, int MaxDrawLength, std::string_view *Options, size_t OptionCount) : Menu(X, Y, Width, MaxDrawLength)
{
    for (size_t i = 0; i < OptionCount; i++)
    {
        m_Options.push_back(Options[i].data());
    }
}

void UI::Menu::AddOption(std::string_view Option)
{
    m_Options.push_back(Option.data());
}

void UI::Menu::Update(void)
{
    int OptionEnd = m_Options.size() - 1;

    if (Input::ButtonPressed(KEY_UP) && --m_Selected < m_OptionStart && --m_OptionStart < 0)
    {
        m_Selected = 0;
        m_OptionStart = 0;
    }
    else if (Input::ButtonPressed(KEY_DOWN) && ++m_Selected > m_OptionStart + m_MaximumDrawLength &&
             ++m_OptionStart + m_MaximumDrawLength > OptionEnd)
    {
        m_OptionStart = OptionEnd - m_MaximumDrawLength;
    }
}

void UI::Menu::Draw(SDL_Surface *Target)
{
    if (m_Options.size() <= 0)
    {
        return;
    }

    for (int i = m_OptionStart, Y = m_Y; i < m_OptionStart + (m_MaximumDrawLength + 1); i++, Y += (12 + 4))
    {
        if (i == m_Selected)
        {
            SDL::DrawRect(Target, m_X - 2, Y - 2, m_Width, 16, {0x00FFFFFF});
        }
        m_Noto->BlitTextAt(Target, m_X, Y, 10, SDL::Font::NO_TEXT_WRAP, m_Options.at(i).c_str());
    }
}

#include "UI/Menu.hpp"
#include "Assets.hpp"
#include "Input.hpp"
#include <3ds.h>

UI::Menu::Menu(int X, int Y, int MaxDrawLength)
    : m_X(X), m_Y(Y), m_MaximumDrawLength(MaxDrawLength),
      m_Noto(SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White))
{
}

UI::Menu::Menu(int X, int Y, int MaxDrawLength, std::string_view *Options, size_t OptionCount) : Menu(X, Y, MaxDrawLength)
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

    if (Input::ButtonPressed(KEY_UP) && --m_Selected < m_OptionStart && m_OptionStart < 0)
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

    for (int i = m_OptionStart; i < m_MaximumDrawLength; i++)
    {
        if (i == m_Selected)
        {
            m_Noto->BlitTextAt(Target, m_X, m_Y * (i - m_OptionStart) + 4, 12, SDL::Font::NO_TEXT_WRAP, "-> %s", m_Options.at(i).c_str());
        }
        else
        {
            m_Noto->BlitTextAt(Target, m_X, m_Y * (i - m_OptionStart) + 4, 12, SDL::Font::NO_TEXT_WRAP, "%s", m_Options.at(i).c_str());
        }
    }
}

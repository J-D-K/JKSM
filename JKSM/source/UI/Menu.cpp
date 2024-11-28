#include "UI/Menu.hpp"
#include "Assets.hpp"
#include "Input.hpp"
#include <3ds.h>

UI::Menu::Menu(int X, int Y, int Width, int MaxDrawLength)
    : m_X(X), m_Y(Y), m_Width(Width), m_MaximumDrawLength(MaxDrawLength - 1),
      m_Noto(SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White))
{
}

void UI::Menu::AddOption(std::string_view Option)
{
    m_Options.push_back(Option.data());
    ++m_OptionsLength;
}

void UI::Menu::Update(void)
{
    int OptionEnd = m_Options.size() - 1;

    if (Input::ButtonPressed(KEY_UP))
    {
        Menu::HandleUpPress();
    }
    else if (Input::ButtonPressed(KEY_DOWN))
    {
        Menu::HandleDownPress();
    }
    else if (Input::ButtonPressed(KEY_LEFT))
    {
        Menu::HandleLeftPress();
    }
    else if (Input::ButtonPressed(KEY_RIGHT))
    {
        Menu::HandleRightPress();
    }
}

void UI::Menu::Draw(SDL_Surface *Target)
{
    if (m_Options.size() <= 0)
    {
        return;
    }

    size_t OptionEnd = 0;
    if (m_OptionStart + m_MaximumDrawLength > m_OptionsLength)
    {
        OptionEnd = m_Options.size();
    }
    else
    {
        OptionEnd = m_MaximumDrawLength + 1;
    }

    for (int i = m_OptionStart, Y = m_Y; i < m_OptionStart + OptionEnd; i++, Y += 16)
    {
        if (i == m_Selected)
        {
            SDL::DrawRect(Target, m_X - 2, Y - 2, m_Width, 16, {0x00FFFFFF});
        }
        m_Noto->BlitTextAt(Target, m_X, Y, 10, m_Noto->NO_TEXT_WRAP, m_Options.at(i).c_str());
    }
}

int UI::Menu::GetSelected(void) const
{
    return m_Selected;
}

void UI::Menu::HandleUpPress(void)
{
    if (--m_Selected < 0)
    {
        m_Selected = 0;
    }
    Menu::UpdateOptionStart();
}

void UI::Menu::HandleDownPress(void)
{
    ++m_Selected;
    if (m_Selected > m_OptionsLength)
    {
        m_Selected = 0;
    }
    Menu::UpdateOptionStart();
}

void UI::Menu::HandleLeftPress(void)
{
    m_Selected -= m_MaximumDrawLength / 2;
    if (m_Selected < 0)
    {
        m_Selected = 0;
    }
    Menu::UpdateOptionStart();
}

void UI::Menu::HandleRightPress(void)
{
    m_Selected += m_MaximumDrawLength / 2;
    if (m_Selected > m_OptionsLength)
    {
        m_Selected = m_OptionsLength;
    }
    Menu::UpdateOptionStart();
}

void UI::Menu::UpdateOptionStart(void)
{
    if (m_OptionStart < m_Selected)
    {
        m_OptionStart = m_Selected;
    }
    else if (m_OptionStart + m_MaximumDrawLength > m_OptionsLength)
    {
        m_OptionStart = m_OptionsLength - m_MaximumDrawLength;
    }
}

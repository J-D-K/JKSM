#include "UI/Menu.hpp"

#include "Assets.hpp"
#include "Input.hpp"
#include "UI/Draw.hpp"
#include "logging/logger.hpp"

#include <3ds.h>
#include <cmath>

UI::Menu::Menu(int X, int Y, int Width, int MaxDrawLength)
    : m_X(X)
    , m_Y(Y)
    , m_Width(Width)
    , m_MaximumDrawLength(MaxDrawLength - 1)
    , m_Noto(SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White))
{
}

void UI::Menu::AddOption(std::string_view Option)
{
    m_Options.push_back(Option.data());
    ++m_OptionsLength;
}

void UI::Menu::EditOption(int Index, std::string_view Option)
{
    if (Index < 0 || Index > m_OptionsLength) { return; }
    m_Options[Index] = Option;
}

void UI::Menu::Reset()
{
    m_Options.clear();
    m_OptionsLength = -1;
}

void UI::Menu::Update()
{
    if (m_Selected > m_OptionsLength) { m_Selected = m_OptionsLength; }

    if (Input::ButtonPressed(KEY_UP)) { Menu::HandleUpPress(); }
    else if (Input::ButtonPressed(KEY_DOWN)) { Menu::HandleDownPress(); }
    else if (Input::ButtonPressed(KEY_LEFT)) { Menu::HandleLeftPress(); }
    else if (Input::ButtonPressed(KEY_RIGHT)) { Menu::HandleRightPress(); }
}

void UI::Menu::Draw(SDL_Surface *Target)
{
    if (m_Options.size() <= 0) { return; }

    if (m_ShiftDirection && (m_ColorShift += 6) >= 0x72) { m_ShiftDirection = false; }
    else if (!m_ShiftDirection && (m_ColorShift -= 3) <= 0) { m_ShiftDirection = true; }

    size_t OptionEnd = 0;
    if (m_OptionStart + m_MaximumDrawLength > m_OptionsLength) { OptionEnd = m_Options.size(); }
    else { OptionEnd = m_MaximumDrawLength + 1; }

    for (size_t i = m_OptionStart, Y = m_Y; i < m_OptionStart + OptionEnd; i++, Y += 17)
    {
        if (m_Selected == static_cast<int>(i)) { UI::DrawBoundingBox(Target, m_X - 3, Y - 3, m_Width + 6, 20, m_ColorShift); }
        m_Noto->BlitTextAt(Target, m_X + 3, Y, 12, m_Noto->NO_TEXT_WRAP, m_Options[i].c_str());
    }
}

int UI::Menu::GetSelected() const { return m_Selected; }

size_t UI::Menu::GetSize() const { return m_Options.size(); }

void UI::Menu::HandleUpPress()
{
    --m_Selected;
    if (m_Selected < 0)
    {
        m_Selected    = m_OptionsLength;
        m_OptionStart = m_MaximumDrawLength > m_OptionsLength ? 0 : m_OptionsLength - m_MaximumDrawLength;
    }
    else if (m_Selected < m_OptionStart + (m_MaximumDrawLength / 2) && m_OptionStart > 0) { --m_OptionStart; }
}

void UI::Menu::HandleDownPress()
{
    ++m_Selected;
    if (m_Selected > m_OptionsLength)
    {
        m_Selected    = 0;
        m_OptionStart = 0;
    }
    else if (m_Selected > m_OptionStart + (m_MaximumDrawLength / 2) && m_OptionStart + m_MaximumDrawLength < m_OptionsLength)
    {
        ++m_OptionStart;
    }
}

void UI::Menu::HandleLeftPress()
{
    int OptionJumpCount = std::ceil(static_cast<double>(m_MaximumDrawLength) / 2.0f);
    m_Selected -= OptionJumpCount;
    if (m_Selected < 0)
    {
        m_Selected = 0;
        // It's safe to assume this should be 0 too.
        m_OptionStart = 0;
    }
    else if (m_Selected - OptionJumpCount < m_OptionStart)
    {
        m_OptionStart = (m_Selected - OptionJumpCount) <= 0 ? 0 : m_Selected - OptionJumpCount;
    }
}

void UI::Menu::HandleRightPress()
{
    int OptionJumpCount = std::ceil(static_cast<double>(m_MaximumDrawLength) / 2.0f);
    m_Selected += OptionJumpCount;
    if (m_Selected > m_OptionsLength)
    {
        m_Selected    = m_OptionsLength;
        m_OptionStart = m_MaximumDrawLength > m_OptionsLength ? 0 : m_OptionsLength - m_MaximumDrawLength;
    }
    else if (m_Selected > m_OptionStart + m_MaximumDrawLength)
    {
        m_OptionStart = (m_Selected + OptionJumpCount) > m_OptionsLength ? m_OptionsLength - m_MaximumDrawLength
                                                                         : m_Selected - OptionJumpCount;
    }
}

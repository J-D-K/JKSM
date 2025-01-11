#include "AppStates/MessageState.hpp"
#include "Input.hpp"
#include "SDL/SDL.hpp"
#include "UI/Draw.hpp"
#include "UI/Strings.hpp"

MessageState::MessageState(AppState *CreatingState, std::string_view MessageString)
    : m_CreatingState(CreatingState), m_MessageString(MessageString)
{
    m_OKX = 160 - (m_Noto->GetTextWidth(12, UI::Strings::GetStringByName(UI ::Strings::Names::OK, 0)) / 2);
}

void MessageState::Update(void)
{
    // Basically any button pressed.
    if (Input::ButtonPressed(0xFFFFFFFF))
    {
        AppState::Deactivate();
    }
}

void MessageState::DrawTop(SDL_Surface *Target)
{
    if (m_CreatingState)
    {
        m_CreatingState->DrawTop(Target);
    }
}

void MessageState::DrawBottom(SDL_Surface *Target)
{
    if (m_CreatingState)
    {
        m_CreatingState->DrawBottom(Target);
    }

    // Dialog
    UI::DrawDialogBox(Target, 8, 18, 304, 204);
    // Message
    m_Noto->BlitTextAt(Target, 30, 30, 12, 268, m_MessageString.c_str());
    // Fake ok button thingy.
    SDL::DrawRect(Target, 8, 190, 304, 1, SDL::Colors::White);
    // OK text
    m_Noto->BlitTextAt(Target, m_OKX, 192, 12, m_Noto->NO_TEXT_WRAP, UI::Strings::GetStringByName(UI::Strings::Names::OK, 0));
}

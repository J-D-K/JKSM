#include "appstates/MessageState.hpp"

#include "Input.hpp"
#include "SDL/SDL.hpp"
#include "Strings.hpp"
#include "UI/Draw.hpp"

MessageState::MessageState(BaseState *creatingState, std::string_view message)
    : m_creatingState(creatingState)
    , m_message(message)
{
    const char *ok = Strings::GetStringByName(Strings::Names::OK, 0);
    m_okX          = 160 - (m_noto->GetTextWidth(12, ok) / 2);
}

void MessageState::update()
{
    // Basically any button pressed.
    if (Input::ButtonPressed(0xFFFFFFFF)) { BaseState::deactivate(); }
}

void MessageState::draw_top(SDL_Surface *target)
{
    if (m_creatingState) { m_creatingState->draw_top(target); }
}

void MessageState::draw_bottom(SDL_Surface *target)
{
    if (m_creatingState) { m_creatingState->draw_bottom(target); }

    // Dialog
    UI::DrawDialogBox(target, 8, 18, 304, 204);
    // Message
    m_noto->BlitTextAt(target, 30, 30, 12, 268, m_message.c_str());
    // Fake ok button thingy.
    SDL::DrawRect(target, 8, 190, 304, 1, SDL::Colors::White);
    // OK text
    m_noto->BlitTextAt(target, m_okX, 200, 12, m_noto->NO_TEXT_WRAP, Strings::GetStringByName(Strings::Names::OK, 0));
}

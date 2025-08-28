#include "appstates/BaseState.hpp"

#include "Assets.hpp"

BaseState::BaseState(StateFlags type)
    : m_stateType(type)
{
    if (m_noto) { return; }
    m_noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);
}

bool BaseState::is_active() const { return m_isActive; }

bool BaseState::has_focus() const { return m_hasFocus; }

void BaseState::deactivate() { m_isActive = false; }

void BaseState::give_focus() { m_hasFocus = true; }

void BaseState::take_focus() { m_hasFocus = false; }

BaseState::StateFlags BaseState::get_type() const { return m_stateType; }
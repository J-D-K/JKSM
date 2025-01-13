#pragma once
#include "Assets.hpp"
#include "SDL/SDL.hpp"

class AppState
{
    public:
        // This is needed so JKSM knows whether or not to allow exiting or state changing.
        // Semi-lock allows closing JKSM, but not state changing.
        // Full lock means neither.
        enum class StateFlags
        {
            Normal,
            SemiLock,
            Lock
        };

        AppState(StateFlags Type = StateFlags::Normal)
            : m_StateType(Type),
              m_Noto(SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White)) {};

        virtual ~AppState() {};
        virtual void Update(void) = 0;
        virtual void DrawTop(SDL_Surface *Target) = 0;
        virtual void DrawBottom(SDL_Surface *Target) = 0;

        // I usually avoid having this stuff in headers now, but here I don't care.
        bool IsActive(void) const
        {
            return m_IsActive;
        }

        bool HasFocus(void) const
        {
            return m_HasFocus;
        }

        void Deactivate(void)
        {
            m_IsActive = false;
        }

        void TakeFocus(void)
        {
            m_HasFocus = false;
        }

        void GiveFocus(void)
        {
            m_HasFocus = true;
        }

        AppState::StateFlags GetType(void) const
        {
            return m_StateType;
        }

    private:
        // Whether state is active or can be purged.
        bool m_IsActive = true;
        // Whether a state is at the back of the vector and has focus.
        bool m_HasFocus = false;
        // Stores what type of state
        AppState::StateFlags m_StateType;

    protected:
        // Pretty much every state needs this so.
        SDL::SharedFont m_Noto = nullptr;
};

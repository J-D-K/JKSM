#include "AppStates/TitleOptionState.hpp"
#include "FS/FS.hpp"
#include "Input.hpp"
#include "UI/Draw.hpp"
#include "UI/Strings.hpp"

namespace
{
    enum
    {
        DELETE_SECURE_VALUE,
        ERASE_SAVE_DATA,
        FORMAT_SAVE_DATA
    };
}

TitleOptionState::TitleOptionState(AppState *CreatingState, const Data::TitleData *TargetTitle)
    : m_CreatingState(CreatingState), m_TargetTitle(TargetTitle), m_OptionsMenu(70, 30, 258, 11)
{
    int CurrentString = 0;
    const char *MenuString = nullptr;
    while ((MenuString = UI::Strings::GetStringByName(UI::Strings::Names::TitleOptions, CurrentString++)) != nullptr)
    {
        m_OptionsMenu.AddOption(MenuString);
    }
}

void TitleOptionState::Update(void)
{
    m_OptionsMenu.Update();

    if (Input::ButtonPressed(KEY_A))
    {
        switch (m_OptionsMenu.GetSelected())
        {
            case DELETE_SECURE_VALUE:
            {
                FS::DeleteSecureValue(m_TargetTitle->GetUniqueID());
            }
            break;

            case ERASE_SAVE_DATA:
            {
            }
        }
    }
    else if (Input::ButtonPressed(KEY_B))
    {
        AppState::Deactivate();
    }
}

void TitleOptionState::DrawTop(SDL_Surface *Target)
{
    if (m_CreatingState)
    {
        m_CreatingState->DrawTop(Target);
    }
    // Render dialog & menu
    UI::DrawDialogBox(Target, 48, 18, 304, 204);
    m_OptionsMenu.Draw(Target);
}

void TitleOptionState::DrawBottom(SDL_Surface *Target)
{
    if (m_CreatingState)
    {
        m_CreatingState->DrawBottom(Target);
    }
}

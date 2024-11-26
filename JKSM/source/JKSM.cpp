#include "JKSM.hpp"
#include "AppStates/SettingsState.hpp"
#include "AppStates/TaskState.hpp"
#include "AppStates/TextTitleSelect.hpp"
#include "AppStates/TitleSelectionState.hpp"
#include "Assets.hpp"
#include "Data/Data.hpp"
#include "FS/FS.hpp"
#include "FsLib.hpp"
#include "Input.hpp"
#include "Logger.hpp"
#include "SDL/SDL.hpp"
#include <3ds.h>
#include <mutex>
#include <string_view>
#include <vector>

// This macro cleans stuff up a lot IMO, so I'm going to use it.
#define ABORT_ON_FAILURE(x)                                                                                                                    \
    if (!x)                                                                                                                                    \
    {                                                                                                                                          \
        return;                                                                                                                                \
    }

namespace
{
    // This is the title text and its centered X coordinate.
    constexpr std::string_view TITLE_TEXT = "JK's Save Manager - 11/25/2024";
    // This is to make centering this easier.
    int s_TitleTextX = 0;

    // This is for whether or not JKSM is running.
    bool s_IsRunning = false;
    // Whether or not a refresh of states is required.
    bool s_RefreshRequired = false;

    /*
        This is the font JKSM uses for the entire app. It's shared and used among all states.
        If you would like to replace it, I've include the source to the program to apply a quick
        zlib compress on TTF fonts. You'll need cmake and zlib installed to do so. Once that is done,
        replace the file in the romfs folder and update Assets.hpp to reflect your changes.
    */
    SDL::SharedFont s_Noto = nullptr;

    // Number of AppStates JKSM has. + 1 for settings menu.
    constexpr int APP_STATE_TOTAL = Data::SaveTypeTotal + 1;
    // Array of appstates we can push from.
    std::array<std::shared_ptr<AppState>, APP_STATE_TOTAL> s_AppStateArray = {nullptr};
    // Vector of AppStates
    std::vector<std::shared_ptr<AppState>> s_AppStateVector;
    // Current state we're on
    int s_CurrentState = 0;
} // namespace

// This function makes it easier to log init errors for services.
template <typename... Args>
bool IntializeService(Result (*Function)(Args...), const char *ServiceName, Args... Arguments)
{
    Result InitError = (*Function)(Arguments...);
    if (R_FAILED(InitError))
    {
        Logger::Log("Error initializing %s: 0x%08X.", ServiceName, InitError);
        return false;
    }
    return true;
}

void JKSM::Initialize(void)
{
    // FsLib is needed for almost everything, so it's first.
    ABORT_ON_FAILURE(FsLib::Initialize())

    // All this does is take care of the directories. Everything is all FsLib now.
    FS::Initialize();

    // This will create the log since FsLib init'd
    Logger::Initialize();

    // These are the services JKSM needs
    ABORT_ON_FAILURE(IntializeService(amInit, "AM"));
    ABORT_ON_FAILURE(IntializeService(aptInit, "APT"));
    ABORT_ON_FAILURE(IntializeService(cfguInit, "CFGU"));
    ABORT_ON_FAILURE(IntializeService(hidInit, "HID"));
    ABORT_ON_FAILURE(IntializeService(romfsInit, "RomFs"));

    // SDL Stuff
    ABORT_ON_FAILURE(SDL::Initialize());
    ABORT_ON_FAILURE(SDL::FreeType::Initialize());

    // Load Font if it wasn't already.
    ABORT_ON_FAILURE(
        (s_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White)));

    // Center the title title
    s_TitleTextX = 200 - (s_Noto->GetTextWidth(12, TITLE_TEXT.data()) / 2);

    // This will create and push the state for each save type.
    for (size_t i = 0; i < Data::SaveTypeTotal; i++)
    {
        s_AppStateArray[i] = std::make_shared<TitleSelectionState>(static_cast<Data::SaveDataType>(i));
    }
    // Settings is last.
    s_AppStateArray[APP_STATE_TOTAL - 1] = std::make_shared<SettingsState>();

    // We only want to push the first one for now. It's blank and will get refreshed after Data::Initialize signals its finished.
    JKSM::PushState(s_AppStateArray[0]);

    // This will spawn the loading thread/state.
    JKSM::PushState(std::make_shared<TaskState>(nullptr, Data::Initialize));

    s_IsRunning = true;
}

void JKSM::Exit(void)
{
    SDL::Exit();
    amExit();
    aptExit();
    cfguExit();
    hidExit();
    romfsExit();
}

bool JKSM::IsRunning(void)
{
    return s_IsRunning;
}

void JKSM::Update(void)
{
    // This needs to be in this very specific order.
    Input::Update();

    // Update the back of the vector so tasks can be purged properly.
    if (!s_AppStateVector.empty())
    {
        s_AppStateVector.back()->Update();
    }

    // Check the back of the vector to purge deactivated states.
    while (!s_AppStateVector.empty() && !s_AppStateVector.back()->IsActive())
    {
        s_AppStateVector.pop_back();
        s_AppStateVector.back()->GiveFocus();
    }

    // If the state in the back is a Task type, don't allow exit or state changing.
    if (s_AppStateVector.back()->GetType() == AppState::StateTypes::Task)
    {
        return;
    }

    // If Data::Initialize signals a refresh or a card is inserted, refresh all save type states.
    if (s_RefreshRequired || Data::GameCardUpdateCheck())
    {
        for (size_t i = 0; i < APP_STATE_TOTAL - 1; i++)
        {
            std::static_pointer_cast<TitleSelectionState>(s_AppStateArray.at(i))->Refresh();
        }
        s_RefreshRequired = false;
    }

    // Controls.
    if (Input::ButtonPressed(KEY_START))
    {
        s_IsRunning = false;
    }
    else if (Input::ButtonPressed(KEY_L))
    {
        if (--s_CurrentState < 0)
        {
            s_CurrentState = APP_STATE_TOTAL - 1;
            for (size_t i = 0; i < APP_STATE_TOTAL; i++)
            {
                JKSM::PushState(s_AppStateArray[i]);
            }
        }
        else
        {
            s_AppStateVector.pop_back();
            s_AppStateVector.back()->GiveFocus();
        }
    }
    else if (Input::ButtonPressed(KEY_R))
    {
        if (++s_CurrentState == APP_STATE_TOTAL)
        {
            s_CurrentState = 0;
            s_AppStateVector.erase(s_AppStateVector.begin() + 1, s_AppStateVector.end());
        }
        else
        {
            s_AppStateVector.back()->TakeFocus();
            s_AppStateVector.push_back(s_AppStateArray[s_CurrentState]);
            s_AppStateVector.back()->GiveFocus();
        }
    }
}

// This needs to be thread safe. That's why it looks like this.
void JKSM::Render(void)
{
    SDL::FrameBegin();

    // Top screen
    SDL_Surface *TopScreen = SDL::GetCurrentBuffer();
    s_AppStateVector.back()->DrawTop(TopScreen);
    SDL::DrawRect(TopScreen, 0, 0, 400, 16, SDL::Colors::BarColor);
    s_Noto->BlitTextAt(TopScreen, s_TitleTextX, 1, 12, SDL::Font::NO_TEXT_WRAP, TITLE_TEXT.data());

    SDL::FrameChangeScreens();

    // Bottom screen.
    SDL_Surface *BottomScreen = SDL::GetCurrentBuffer();
    s_AppStateVector.back()->DrawBottom(BottomScreen);
    SDL::DrawRect(BottomScreen, 0, 224, 320, 16, SDL::Colors::BarColor);

    // Present to screen.
    SDL::FrameEnd();
}

void JKSM::PushState(std::shared_ptr<AppState> NewState)
{
    if (!s_AppStateVector.empty())
    {
        s_AppStateVector.back()->TakeFocus();
    }
    NewState->GiveFocus();
    s_AppStateVector.push_back(NewState);
}

void JKSM::RefreshSaveTypeStates(void)
{
    s_RefreshRequired = true;
}

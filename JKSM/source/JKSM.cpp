#include "JKSM.hpp"
#include "AppStates/TitleSelectionState.hpp"
#include "Assets.hpp"
#include "Data/Data.hpp"
#include "FS/FS.hpp"
#include "FsLib.hpp"
#include "Input.hpp"
#include "Logger.hpp"
#include "SDL/SDL.hpp"
#include <3ds.h>
#include <string_view>
#include <vector>

namespace
{
    // This is for whether or not JKSM is running.
    bool m_IsRunning = false;
    // This is the font
    SDL::SharedFont s_Noto = nullptr;
    // This is the title text and its centered X coordinate.
    constexpr std::string_view TITLE_TEXT = "JK's Save Manager - 11/22/2024";
    int s_TitleTextX = 0;
    // Vector of AppStates
    std::vector<std::shared_ptr<AppState>> s_AppStateVector;
    // Array of TitleSelectionStates for each save type that can be auto created.
    std::array<std::shared_ptr<AppState>, Data::SaveTypeTotal - 1> s_TitleSelectionStateArray = {nullptr};
    // Current state we're on
    size_t s_CurrentState = 1;
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

// This function initializes the states for the different save types.
static void InitializeSaveAppStates(void)
{
    // Clear the vector first.
    s_AppStateVector.clear();

    // Initialize states. Since we're using smart pointers we don't need to worry about leaks.
    for (int i = 0; i < Data::SaveTypeTotal - 1; i++)
    {
        s_TitleSelectionStateArray[i] = std::make_shared<TitleSelectionState>(static_cast<Data::SaveDataType>(i));
    }

    // Loop and replace what we were on previously. The end user won't even know this happened.
    for (size_t i = 0; i < s_CurrentState; i++)
    {
        JKSM::PushState(s_TitleSelectionStateArray[i]);
    }
}

void JKSM::Initialize(void)
{
    // FsLib is needed for almost everything, so it's first.
    if (!FsLib::Initialize())
    {
        return;
    }

    // All this does is take care of the directories. Everything is all FsLib now.
    FS::Initialize();

    // This will create the log since FsLib init'd
    Logger::Initialize();

    // These are the services JKSM needs
    if (!IntializeService(amInit, "AM"))
    {
        return;
    }

    if (!IntializeService(aptInit, "APT"))
    {
        return;
    }

    if (!IntializeService(cfguInit, "CFGU"))
    {
        return;
    }

    if (!IntializeService(hidInit, "HID"))
    {
        return;
    }

    if (!IntializeService(romfsInit, "RomFS"))
    {
        return;
    }

    if (!SDL::Initialize())
    {
        return;
    }

    // This isn't part of SDL at all, but I don't really want a whole header and source file for one thing....
    if (!SDL::FreeType::Initialize())
    {
        return;
    }

    if (!Data::Initialize())
    {
        return;
    }

    // Load Font if it wasn't already.
    s_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);
    if (!s_Noto)
    {
        Logger::Log("Error loading Noto sans.");
        return;
    }

    // Center the title title
    s_TitleTextX = 200 - (s_Noto->GetTextWidth(12, TITLE_TEXT.data()) / 2);

    InitializeSaveAppStates();

    m_IsRunning = true;
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
    return m_IsRunning;
}

void JKSM::Update(void)
{
    Input::Update();
    if (Input::ButtonPressed(KEY_START))
    {
        m_IsRunning = false;
    }
    else if (Input::ButtonPressed(KEY_CPAD_LEFT) && s_AppStateVector.size() > 1)
    {
        s_AppStateVector.pop_back();
        s_AppStateVector.back()->GiveFocus();
        s_CurrentState--;
    }
    else if (Input::ButtonPressed(KEY_CPAD_RIGHT))
    {
        s_AppStateVector.back()->TakeFocus();
        s_AppStateVector.push_back(s_TitleSelectionStateArray[s_CurrentState++]);
        s_AppStateVector.back()->GiveFocus();
    }

    // If a card was inserted and successfully read update title selection states.
    if (Data::GameCardUpdateCheck())
    {
        InitializeSaveAppStates();
    }

    while (!s_AppStateVector.empty() && !s_AppStateVector.back()->IsActive())
    {
        s_AppStateVector.pop_back();
        s_AppStateVector.back()->GiveFocus();
    }
    s_AppStateVector.back()->Update();
}

void JKSM::Render(void)
{
    SDL::FrameBegin();
    SDL_Surface *TopScreen = SDL::GetCurrentBuffer();
    s_AppStateVector.back()->DrawTop(TopScreen);
    SDL::DrawRect(TopScreen, 0, 0, 400, 16, SDL::Colors::BarColor);
    s_Noto->BlitTextAt(TopScreen, s_TitleTextX, 1, 12, SDL::Font::NO_TEXT_WRAP, TITLE_TEXT.data());
    SDL::FrameChangeScreens();
    SDL_Surface *BottomScreen = SDL::GetCurrentBuffer();
    s_AppStateVector.back()->DrawBottom(BottomScreen);
    SDL::DrawRect(BottomScreen, 0, 224, 320, 16, SDL::Colors::BarColor);
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

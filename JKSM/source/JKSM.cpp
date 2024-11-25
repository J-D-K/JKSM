#include "JKSM.hpp"
#include "AppStates/TaskState.hpp"
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
    // Mutex to protect vector from corruption
    std::mutex s_AppStateVectorLock;
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

// This assumes this is only being called on Update and after the mutex is already locked.
static void PurgeDeactivatedStates(void)
{
    for (size_t i = 0; i < s_AppStateVector.size(); i++)
    {
        if (!s_AppStateVector.at(i)->IsActive())
        {
            Logger::Log("Purge @ %u", i);
            s_AppStateVector.erase(s_AppStateVector.begin() + i);
        }
    }
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
    s_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);
    if (!s_Noto)
    {
        Logger::Log("Error loading Noto sans.");
        return;
    }

    // Center the title title
    s_TitleTextX = 200 - (s_Noto->GetTextWidth(12, TITLE_TEXT.data()) / 2);

    // This will spawn the loading state.
    JKSM::PushState(std::make_shared<TaskState>(nullptr, Data::Initialize));

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
        std::scoped_lock<std::mutex> AppStateLock(s_AppStateVectorLock);
        s_AppStateVector.pop_back();
        s_AppStateVector.back()->GiveFocus();
        s_CurrentState--;
    }
    else if (Input::ButtonPressed(KEY_CPAD_RIGHT))
    {
        std::scoped_lock<std::mutex> AppStateLock(s_AppStateVectorLock);
        s_AppStateVector.back()->TakeFocus();
        s_AppStateVector.push_back(s_TitleSelectionStateArray[s_CurrentState++]);
        s_AppStateVector.back()->GiveFocus();
    }

    // If a card was inserted and successfully read update title selection states.
    if (Data::GameCardUpdateCheck())
    {
        JKSM::InitializeAppStates();
    }


    {
        std::scoped_lock<std::mutex> AppStateLock(s_AppStateVectorLock);
        PurgeDeactivatedStates();
        while (!s_AppStateVector.empty() && !s_AppStateVector.back()->IsActive())
        {
            s_AppStateVector.pop_back();
            s_AppStateVector.back()->GiveFocus();
        }

        if (!s_AppStateVector.empty())
        {
            s_AppStateVector.back()->Update();
        }
    }
}

// This needs to be thread safe. That's why it looks like this.
void JKSM::Render(void)
{
    SDL::FrameBegin();
    // Top screen
    SDL_Surface *TopScreen = SDL::GetCurrentBuffer();
    {
        std::scoped_lock<std::mutex> AppStateLock(s_AppStateVectorLock);
        s_AppStateVector.back()->DrawTop(TopScreen);
    }
    SDL::DrawRect(TopScreen, 0, 0, 400, 16, SDL::Colors::BarColor);
    s_Noto->BlitTextAt(TopScreen, s_TitleTextX, 1, 12, SDL::Font::NO_TEXT_WRAP, TITLE_TEXT.data());

    // Bottom screen
    SDL::FrameChangeScreens();
    SDL_Surface *BottomScreen = SDL::GetCurrentBuffer();
    {
        std::scoped_lock<std::mutex> AppStateLock(s_AppStateVectorLock);
        s_AppStateVector.back()->DrawBottom(BottomScreen);
    }
    SDL::DrawRect(BottomScreen, 0, 224, 320, 16, SDL::Colors::BarColor);
    SDL::FrameEnd();
}

void JKSM::PushState(std::shared_ptr<AppState> NewState)
{
    std::scoped_lock<std::mutex> AppStateVectorGuard(s_AppStateVectorLock);

    if (!s_AppStateVector.empty())
    {
        s_AppStateVector.back()->TakeFocus();
    }

    NewState->GiveFocus();
    s_AppStateVector.push_back(NewState);
}

// This function initializes the states for the different save types.
void JKSM::InitializeAppStates(void)
{
    // Initialize states. Since we're using smart pointers we don't need to worry about leaks.
    for (int i = 0; i < Data::SaveTypeTotal - 1; i++)
    {
        Logger::Log("Create %i", i);
        s_TitleSelectionStateArray[i] = std::make_shared<TitleSelectionState>(static_cast<Data::SaveDataType>(i));
    }

    {
        std::scoped_lock<std::mutex> AppStateLock(s_AppStateVectorLock);
        // Loop and replace what we were on previously. The end user won't even know this happened.
        for (size_t i = 0; i < s_CurrentState; i++)
        {
            s_TitleSelectionStateArray.at(i)->TakeFocus();
            s_AppStateVector.push_back(s_TitleSelectionStateArray.at(i));
        }
        s_AppStateVector.back()->GiveFocus();
    }
}

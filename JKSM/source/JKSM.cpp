#include "JKSM.hpp"
#include "AppStates/TitleSelectionState.hpp"
#include "Data/Data.hpp"
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
    // These are the surfaces for the bars on the top and bottom. I was going to stretch, but it kills the framerate and throws the colors off.
    SDL::SharedSurface s_TopBar = nullptr;
    SDL::SharedSurface s_BottomBar = nullptr;
    // This is the font
    SDL::SharedFont s_Noto = nullptr;
    // This is the default color used by the font
    SDL::Color COLOR_WHITE = {0xFFFFFFFF};
    // This is the title text and its centered X coordinate.
    constexpr std::string_view TITLE_TEXT = "JK's Save Manager - 11/22/2024";
    int s_TitleTextX = 0;
    // These are the paths to files JKSM uses.
    // It took almost 10 years but I finally did it.
    constexpr std::u16string_view JKSV_FOLDER = u"sdmc:/JKSV";
    constexpr std::u16string_view JKSM_FOLDER = u"sdmc:/JKSM";
    constexpr std::u16string_view CONFIG_FOLDER = u"sdmc:/config/JKSM";
    // Vector of AppStates
    std::vector<std::shared_ptr<AppState>> s_AppStateVector;
    // Array of TitleSelectionStates for each save type.
    std::array<std::shared_ptr<AppState>, Data::SaveTypeTotal> s_TitleSelectionStateArray = {nullptr};
    // Current state we're on
    size_t s_CurrentState = 0;
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
    if (!FsLib::Initialize())
    {
        return;
    }

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

    /*s_TopBar = SDL::SurfaceManager::CreateLoadResource("TOP_BAR", "romfs:/BarTop.png", false);
    s_BottomBar = SDL::SurfaceManager::CreateLoadResource("BOTTOM_BAR", "romfs:/BarBottom.png", false);
    if (!s_TopBar || !s_BottomBar)
    {
        Logger::Log("Error loading assets.");
        return;
    }*/

    // Load Noto sans
    s_Noto = SDL::FontManager::CreateLoadResource("NotoSans", "romfs:/NotoSansJP-Medium.ttf", COLOR_WHITE);
    if (!s_Noto)
    {
        Logger::Log("Error loading Noto sans.");
        return;
    }

    // Create config and JKSM folder.
    if (!FsLib::DirectoryExists(CONFIG_FOLDER) && !FsLib::CreateDirectoriesRecursively(CONFIG_FOLDER))
    {
        Logger::Log("Error creating config folder: %s", FsLib::GetErrorString());
        return;
    }

    if (FsLib::DirectoryExists(JKSV_FOLDER) && !FsLib::RenameDirectory(JKSV_FOLDER, JKSM_FOLDER))
    {
        // Not going to fail on this.
        Logger::Log("Rename JKSV to JKSM: %s", FsLib::GetErrorString());
    }

    if (!FsLib::DirectoryExists(JKSM_FOLDER) && !FsLib::CreateDirectoriesRecursively(JKSM_FOLDER))
    {
        // This actually is realling important.
        Logger::Log("Error creating JKSM folder: %s", FsLib::GetErrorString());
        return;
    }

    // Center the title title
    s_TitleTextX = 200 - (s_Noto->GetTextWidth(12, TITLE_TEXT.data()) / 2);

    // Create states for each title type.
    for (size_t i = 0; i < Data::SaveTypeTotal; i++)
    {
        s_TitleSelectionStateArray[i] = std::make_shared<TitleSelectionState>(static_cast<Data::SaveDataType>(i));
    }

    // We start with 0
    JKSM::PushState(s_TitleSelectionStateArray[0]);

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

    if (Input::ButtonPressed(KEY_CPAD_LEFT) && s_AppStateVector.size() > 1)
    {
        s_AppStateVector.pop_back();
        s_AppStateVector.back()->GiveFocus();
        s_CurrentState--;
    }
    else if (Input::ButtonPressed(KEY_CPAD_RIGHT))
    {
        s_AppStateVector.back()->TakeFocus();
        s_AppStateVector.push_back(s_TitleSelectionStateArray[++s_CurrentState]);
        s_AppStateVector.back()->GiveFocus();
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
    SDL::DrawRect(TopScreen, 0, 0, 400, 16, {0x3D3D3DFF});
    s_Noto->BlitTextAt(TopScreen, s_TitleTextX, 1, 12, SDL::Font::NO_TEXT_WRAP, TITLE_TEXT.data());
    SDL::FrameChangeScreens();
    SDL_Surface *BottomScreen = SDL::GetCurrentBuffer();
    s_AppStateVector.back()->DrawBottom(BottomScreen);
    SDL::DrawRect(BottomScreen, 0, 224, 320, 16, {0x3D3D3DFF});
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

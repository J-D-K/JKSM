#include "JKSM.hpp"

#include "AppStates/MessageState.hpp"
#include "AppStates/ProgressTaskState.hpp"
#include "AppStates/SettingsState.hpp"
#include "AppStates/TextTitleSelect.hpp"
#include "AppStates/TitleSelectionState.hpp"
#include "Assets.hpp"
#include "Config.hpp"
#include "Data/Data.hpp"
#include "FS/FS.hpp"
#include "Input.hpp"
#include "Keyboard.hpp"
#include "SDL/SDL.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"
#include "logging/logger.hpp"

#include <3ds.h>
#include <mutex>
#include <string_view>
#include <vector>

// This macro cleans stuff up a lot IMO, so I'm going to use it.
#define ABORT_ON_FAILURE(x)                                                                                                    \
    if (!x) { return; }

namespace
{
    // This is the title text.
    constexpr std::string_view TITLE_TEXT = "JK's Save Manager - 01/13/2025";

    // This is the device name used to set play coins.
    constexpr std::u16string_view SHARED_DEVICE = u"shared";
} // namespace

// This function makes it easier to log init errors for services.
template <typename... Args>
bool IntializeService(Result (*Function)(Args...), const char *ServiceName, Args... Arguments)
{
    Result InitError = (*Function)(Arguments...);
    if (R_FAILED(InitError))
    {
        logger::log("Error initializing %s: 0x%08X.", ServiceName, InitError);
        return false;
    }
    return true;
}

JKSM::JKSM(void)
{
    // FsLib is needed the most, so it's first.
    ABORT_ON_FAILURE(fslib::initialize());

    // Bypass archive_dev.
    ABORT_ON_FAILURE(fslib::dev::initialize_sdmc());

    // This takes care of making sure needed directories exist.
    FS::Initialize();

    // Creates and clears log
    logger::initialize();

    // Services JKSM  needs.
    ABORT_ON_FAILURE(IntializeService(amInit, "AM"));
    ABORT_ON_FAILURE(IntializeService(aptInit, "APT"));
    ABORT_ON_FAILURE(IntializeService(cfguInit, "CFGU"));
    ABORT_ON_FAILURE(IntializeService(hidInit, "HID"));
    ABORT_ON_FAILURE(IntializeService(romfsInit, "RomFs"));

    // Check for New 3DS and enable clock & L2
    bool New3DS     = false;
    Result AptError = APT_CheckNew3DS(&New3DS);
    if (R_SUCCEEDED(AptError) && New3DS) { osSetSpeedupEnable(true); }

    // SDL & Freetype.
    ABORT_ON_FAILURE(SDL::Initialize());
    ABORT_ON_FAILURE(SDL::FreeType::Initialize());

    // Config
    Config::Initialize();

    // Loads UI strings from json in RomFs.
    Strings::Intialize();

    // Load and decompress font and use white as the default color.
    // JKSM can't change colors like JKSV can on Switch unfortunately. SDL 3DS is a CPU/soft rendered with surfaces and the
    // working needed and extra processing power isn't worth it.
    m_Noto = SDL::FontManager::CreateLoadResource(Asset::Names::NOTO_SANS, Asset::Paths::NOTO_SANS_PATH, SDL::Colors::White);
    ABORT_ON_FAILURE(m_Noto);

    // Center the title text.
    m_TitleTextX = 200 - (m_Noto->GetTextWidth(12, TITLE_TEXT.data()) / 2);

    // Align L and R. There's something off about the left arrow glyph in the Noto Sans font...
    m_LX = m_Noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::LR, 0)) / 3;
    m_RX = 392 - m_Noto->GetTextWidth(12, Strings::GetStringByName(Strings::Names::LR, 1));

    // Init title select views.
    JKSM::InitializeViews();

    // Create settings in final array index.
    m_StateArray[m_StateTotal - 1] = std::make_shared<SettingsState>();

    // Push the data loading state.
    JKSM::PushState(std::make_shared<ProgressTaskState>(nullptr, Data::Initialize));

    // Should be good to go assuming data loading thread doesn't explode.
    m_IsRunning = true;
}

JKSM::~JKSM()
{
    SDL::Exit();
    SDL::FreeType::Exit();
    romfsExit();
    hidExit();
    cfguExit();
    aptExit();
    amExit();
}

bool JKSM::IsRunning(void) const { return m_IsRunning; }

void JKSM::Update(void)
{
    Input::Update();

    // This needs to be done here so the purging loop works correctly.
    if (!m_StateStack.empty()) { m_StateStack.top()->Update(); }

    // Pop from stack until active state is hit. Make sure it has focus.
    while (!m_StateStack.empty() && !m_StateStack.top()->IsActive())
    {
        m_StateStack.pop();
        m_StateStack.top()->GiveFocus();
    }

    // If the back is a locking type state, bail and don't allow exiting with start.
    if (m_StateStack.top()->GetType() == AppState::StateFlags::Lock) { return; }

    // If a refresh is signaled or a cart is inserted.
    if (m_RefreshRequired || Data::GameCardUpdateCheck())
    {
        // Loop and refresh all view states in array.
        for (size_t i = 0; i < m_StateTotal - 1; i++)
        {
            std::static_pointer_cast<TitleSelectionState>(m_StateArray.at(i))->Refresh();
        }
        m_RefreshRequired = false;
    }

    // Global JKSM controls.
    if (Input::ButtonPressed(KEY_START)) { m_IsRunning = false; } // Only allow state switching if the top isn't a semi-lock
    else if (Input::ButtonPressed(KEY_L) && m_StateStack.top()->GetType() != AppState::StateFlags::SemiLock)
    {
        if (--m_CurrentState < 0)
        {
            // Set current state to max
            m_CurrentState = m_StateTotal - 1;
            // Loop and push entire array to stack,
            for (size_t i = 0; i < m_StateTotal; i++) { JKSM::PushState(m_StateArray.at(i)); }
        }
        else
        {
            m_StateStack.pop();
            m_StateStack.top()->GiveFocus();
        }
    }
    else if (Input::ButtonPressed(KEY_R) && m_StateStack.top()->GetType() != AppState::StateFlags::SemiLock)
    {
        if (++m_CurrentState == m_StateTotal)
        {
            m_CurrentState = 0;
            // Pop all states except the first.
            for (size_t i = m_StateTotal; i > 1; i--) { m_StateStack.pop(); }
            m_StateStack.top()->GiveFocus();
        }
        else
        {
            m_StateStack.top()->TakeFocus();
            m_StateStack.push(m_StateArray[m_CurrentState]);
            m_StateStack.top()->GiveFocus();
        }
    }
    else if (Input::ButtonPressed(KEY_SELECT) && m_StateStack.top()->GetType() != AppState::StateFlags::SemiLock)
    {
        JKSM::SetPlayCoins();
    }
}

void JKSM::Draw(void)
{
    SDL::FrameBegin();

    // Get top screen surface
    SDL_Surface *Top = SDL::GetCurrentBuffer();
    // Draw the top of the stack
    if (!m_StateStack.empty()) { m_StateStack.top()->DrawTop(Top); }
    // Draw the top bar, title and <L R>
    SDL::DrawRect(Top, 0, 0, 400, 16, SDL::Colors::BarColor);
    m_Noto->BlitTextAt(Top, m_TitleTextX, 1, 12, m_Noto->NO_TEXT_WRAP, TITLE_TEXT.data());
    m_Noto->BlitTextAt(Top, m_LX, 225, 12, m_Noto->NO_TEXT_WRAP, Strings::GetStringByName(Strings::Names::LR, 0));
    m_Noto->BlitTextAt(Top, m_RX, 225, 12, m_Noto->NO_TEXT_WRAP, Strings::GetStringByName(Strings::Names::LR, 1));

    SDL::FrameChangeScreens();

    // Get bottom screen surface.
    SDL_Surface *Bottom = SDL::GetCurrentBuffer();
    if (!m_StateStack.empty()) { m_StateStack.top()->DrawBottom(Bottom); }
    // Just draw the bottom bar here.
    SDL::DrawRect(Bottom, 0, 224, 320, 16, SDL::Colors::BarColor);

    SDL::FrameEnd();
}

void JKSM::PushState(std::shared_ptr<AppState> NewState)
{
    if (!m_StateStack.empty()) { m_StateStack.top()->TakeFocus(); }

    // Give focus to incoming state.
    NewState->GiveFocus();
    // Push it to stack
    m_StateStack.push(NewState);
}

void JKSM::RefreshViews(void) { m_RefreshRequired = true; }

void JKSM::InitializeViews(void)
{
    // Whether or not we're using text mode.
    bool TextMode = Config::GetByKey(Config::Keys::TextMode);

    // Loop and create the states.
    for (size_t i = 0; i < m_StateTotal - 1; i++)
    {
        if (TextMode) { m_StateArray[i] = std::make_shared<TextTitleSelect>(static_cast<Data::SaveDataType>(i)); }
        else { m_StateArray[i] = std::make_shared<TitleSelectionState>(static_cast<Data::SaveDataType>(i)); }
    }

    // Clear stack.
    for (size_t i = 0; i < m_StateStack.size(); i++) { m_StateStack.pop(); }

    // Push states until the current is hit again.
    for (int i = 0; i <= m_CurrentState; i++) { m_StateStack.push(m_StateArray.at(i)); }
}

void JKSM::SetPlayCoins(void)
{
    if (!fslib::open_shared_extra_data(SHARED_DEVICE, 0xF000000B))
    {
        ShowMessage(m_StateStack.top().get(), Strings::GetStringByName(Strings::Names::PlayCoinsMessages, 0));
        return;
    }

    // Open target file.
    fslib::File GameCoin(u"shared:/gamecoin.dat", FS_OPEN_READ | FS_OPEN_WRITE);
    if (!GameCoin.is_open())
    {
        fslib::close_device(SHARED_DEVICE);
        ShowMessage(m_StateStack.top().get(), Strings::GetStringByName(Strings::Names::PlayCoinsMessages, 1));
        return;
    }

    // seek to position and read the current play coin amount.
    uint16_t CurrentPlayCoins = 0, DesiredPlayCoins = 0;
    GameCoin.seek(0x4, GameCoin.BEGINNING);
    GameCoin.read(&CurrentPlayCoins, sizeof(uint16_t));

    if (!Keyboard::GetUnsignedIntWithKeyboard(CurrentPlayCoins, reinterpret_cast<unsigned int *>(&DesiredPlayCoins)))
    {
        // Just cleanup and return on cancel.
        fslib::close_device(SHARED_DEVICE);
        return;
    }

    // seek to same position as before.
    GameCoin.seek(0x4, GameCoin.BEGINNING);

    // Write desired amount.
    GameCoin.write(&DesiredPlayCoins, sizeof(uint16_t));

    // Cleanup
    fslib::close_device(SHARED_DEVICE);

    // Show message
    std::string CoinSuccess =
        StringUtil::GetFormattedString(Strings::GetStringByName(Strings::Names::PlayCoinsMessages, 2), DesiredPlayCoins);
    ShowMessage(m_StateStack.top().get(), CoinSuccess);
}

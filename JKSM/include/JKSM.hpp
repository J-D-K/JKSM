#pragma once
#include "AppStates/AppState.hpp"
#include "Data/Data.hpp"
#include <array>
#include <memory>
#include <stack>

class JKSM
{
    public:
        // No copying. There should only every be one instance of JKSM.
        JKSM(const JKSM &) = delete;
        JKSM(JKSM &&) = delete;
        JKSM &operator=(const JKSM &) = delete;
        JKSM &operator=(JKSM &&) = delete;
        // Constructor initializes services and a few needed things.
        JKSM(void);
        // Exits services and shuts down.
        ~JKSM();
        // Returns whether initialization was successful and JKSM is running.
        bool IsRunning(void) const;
        // Runs the main update loop function.
        void Update(void);
        // Draws the top and bottom screens.
        void Draw(void);
        // Pushes a new state to the stack.
        static void PushState(std::shared_ptr<AppState> NewState);
        // Forces JKSM to refresh title views the next time update is called.
        static void RefreshViews(void);
        // Forces JKSM to reinitialize all title views on next Update()
        static void InitializeViews(void);

    private:
        // To center title text.
        int m_TitleTextX = 0;
        // X positions of <L and R>
        int m_LX = 0, m_RX = 0;
        // Whether or not initialization was successful.
        bool m_IsRunning = false;
        // Font used to draw all text in JKSM. This is shared across the app and this *should* always be the first thing to load it.
        SDL::SharedFont m_Noto = nullptr;
        // Current state JKSM is using.
        static inline int m_CurrentState = 0;
        // Whether or not a state refresh is needed on next Update()
        static inline bool m_RefreshRequired = false;
        // Total number of states JKSM has. Basically total number of save types + 1 for settings.
        static inline constexpr int m_StateTotal = Data::SaveTypeTotal + 1;
        // Array of states we push from. + 1 for settings
        static inline std::array<std::shared_ptr<AppState>, m_StateTotal> m_StateArray = {nullptr};
        // Stack of states we push and pop from.
        static inline std::stack<std::shared_ptr<AppState>> m_StateStack;
};

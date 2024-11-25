#pragma once
#include "AppStates/AppState.hpp"
#include <memory>

namespace JKSM
{
    // Initializes services, FsLib and SDL. Doesn't return anything because IsRunning() will make the loop fall through anyway.
    void Initialize(void);
    // Exits services.
    void Exit(void);
    // Returns if JKSM is still running.
    bool IsRunning(void);
    // Updates input and back of state vector
    void Update(void);
    // "Renders", or blits everything to screen.
    void Render(void);
    // Pushes a new state to the state vector
    void PushState(std::shared_ptr<AppState> NewState);
    // Clears and initializes all JKSM's AppStates.
    void InitializeAppStates(void);
} // namespace JKSM

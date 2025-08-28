#pragma once
#include <cstdint>

namespace input
{
    /// @brief Updates input for the current frame.
    void update();

    /// @brief Returns if the button passed was pressed.
    bool button_pressed(uint32_t button);

    /// @brief Returns if the button passed was held.
    bool button_held(uint32_t button);

    /// @brief Returns if teh button passed was released.
    bool button_released(uint32_t button);
} // namespace Input

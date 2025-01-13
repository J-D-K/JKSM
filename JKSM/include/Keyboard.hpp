#pragma once
#include <string>

namespace Keyboard
{
    // Attempts to get a string with the 3DS software keyboard. Returns true on success, false on failure.
    bool GetStringWithKeyboard(const char *DefaultText, char16_t *TextOut, int TextOutLength);
    // Attempts to get unsigned int using keyboard. Returns true on success, false on failure.
    bool GetUnsignedIntWithKeyboard(unsigned int DefaultValue, unsigned int *Out);
} // namespace Keyboard

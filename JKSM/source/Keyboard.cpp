#include "Keyboard.hpp"
#include "Logger.hpp"
#include "UI/Strings.hpp"
#include <3ds.h>
#include <cstdio>
#include <cstdlib>

bool Keyboard::GetStringWithKeyboard(const char *DefaultText, char16_t *TextOut, int TextOutLength)
{
    SwkbdState KeyboardState;
    // Why does this function say UTF-16 characters, but swkbdInputText writes to char?
    swkbdInit(&KeyboardState, SWKBD_TYPE_QWERTY, 2, TextOutLength);
    swkbdSetInitialText(&KeyboardState, DefaultText);
    swkbdSetButton(&KeyboardState, SWKBD_BUTTON_LEFT, UI::Strings::GetStringByName(UI::Strings::Names::KeyboardButtons, 0), false);
    swkbdSetButton(&KeyboardState, SWKBD_BUTTON_RIGHT, UI::Strings::GetStringByName(UI::Strings::Names::KeyboardButtons, 1), true);

    char UTF8Buffer[TextOutLength + 1] = {0};
    SwkbdButton Button = swkbdInputText(&KeyboardState, UTF8Buffer, TextOutLength);
    if (Button == SWKBD_BUTTON_LEFT)
    {
        return false;
    }
    // Probably convert back to what it originally was...
    utf8_to_utf16(reinterpret_cast<uint16_t *>(TextOut), reinterpret_cast<const uint8_t *>(UTF8Buffer), TextOutLength);
    return true;
}

bool Keyboard::GetUnsignedIntWithKeyboard(unsigned int DefaultValue, unsigned int *Out)
{
    // Get text for Default
    char NumberTextBuffer[11] = {0};
    std::snprintf(NumberTextBuffer, 11, "%u", DefaultValue);

    // Setup keyboard state.
    SwkbdState KeyboardState;
    swkbdInit(&KeyboardState, SWKBD_TYPE_NUMPAD, 2, 10);
    swkbdSetInitialText(&KeyboardState, NumberTextBuffer);
    swkbdSetButton(&KeyboardState, SWKBD_BUTTON_LEFT, UI::Strings::GetStringByName(UI::Strings::Names::KeyboardButtons, 0), false);
    swkbdSetButton(&KeyboardState, SWKBD_BUTTON_RIGHT, UI::Strings::GetStringByName(UI::Strings::Names::KeyboardButtons, 1), true);

    char NumberBuffer[11] = {0};
    SwkbdButton Button = swkbdInputText(&KeyboardState, NumberBuffer, 10);
    if (Button == SWKBD_BUTTON_LEFT)
    {
        // Cancelled.
        return false;
    }

    // Set out.
    *Out = std::strtoul(NumberBuffer, NULL, 10);

    // Should be fine.
    return true;
}

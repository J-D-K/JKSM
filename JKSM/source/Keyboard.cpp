#include "Keyboard.hpp"
#include "Logger.hpp"
#include "UI/Strings.hpp"
#include <3ds.h>
#include <memory>
#include <minizip/zip.h>

bool GetStringWithKeyboard(char16_t *TextOut, int TextOutLength)
{
    SwkbdState KeyboardState;
    // Why does this function say UTF-16 characters, but swkbdInputText writes to char?
    swkbdInit(&KeyboardState, SWKBD_TYPE_QWERTY, 2, TextOutLength);
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

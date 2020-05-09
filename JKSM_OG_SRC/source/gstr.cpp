#include <string>

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <sf2d.h>

#include "gstr.h"
#include "global.h"
#include "date.h"
#include "ui.h"

std::string GetString(const char *hint)
{
    hidScanInput();

    u32 held = hidKeysHeld();
    if(held & KEY_L)
        return GetDate(FORMAT_YDM);
    else if(held & KEY_R)
        return  GetDate(FORMAT_YMD);

    SwkbdState keyState;
    char input[64];

    swkbdInit(&keyState, SWKBD_TYPE_NORMAL, 2, 64);
    swkbdSetHintText(&keyState, hint);
    swkbdSetFeatures(&keyState, SWKBD_PREDICTIVE_INPUT);
    SwkbdDictWord dates[2];
    swkbdSetDictWord(&dates[0], "2016", GetDate(FORMAT_YDM));
    swkbdSetDictWord(&dates[1], "2016", GetDate(FORMAT_YMD));
    swkbdSetInitialText(&keyState, GetDate(FORMAT_YMD));
    swkbdSetDictionary(&keyState, dates, 2);

    swkbdInputText(&keyState, input, 64);

    return std::string(input);
}

int getInt(const char *hint, int init, int maxValue)
{
    SwkbdState keyState;
    char input[8];

    swkbdInit(&keyState, SWKBD_TYPE_NUMPAD, 2, 8);
    swkbdSetHintText(&keyState, hint);
    if(init != -1)
    {
        sprintf(input, "%i", init);
        swkbdSetInitialText(&keyState, input);
    }

    SwkbdButton pressed = swkbdInputText(&keyState, input, 8);
    int ret;
    //Cancel
    if(pressed == SWKBD_BUTTON_LEFT)
        ret = -1;
    else
    {
        ret = strtol(input, NULL, 10);
        if(ret > maxValue)
            ret = maxValue;
    }
    return ret;
}

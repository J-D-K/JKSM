#include <3ds.h>
#include <stdio.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>
#include <cstdlib>
#include <fstream>

#include "sys.h"
#include "global.h"
#include "util.h"
#include "3dsx.h"
#include "titles.h"

int main(int argc, const char * argv[])
{
    hidInit();
    hidScanInput();
    u32 held = hidKeysHeld();
    if((held & KEY_R) && (held & KEY_L))
        devMode = true;
    //This is for making sure I didn't butcher the font
    else if(held & KEY_R)
        sysLanguage = CFG_LANGUAGE_JP;

    sysInit();

    if(runningUnder() && !devMode)
    {
        hbl = true;
        start3dsxMode();
    }
    else
    {
        sdTitlesInit();
        nandTitlesInit();

        while(aptMainLoop() && !kill)
        {
            handleState();
        }
    }

    sysExit();
    return 0;
}

#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "extra.h"
#include "global.h"
#include "menu.h"
#include "archive.h"
#include "ui.h"
#include "gstr.h"
#include "file.h"

/*These are extra things I added.
I wasn't sure if there was a cia coin setter
besides Nintendo's official one.
*/

void setPlayCoins()
{
    FS_Archive shared;
    if(openSharedExt(&shared, 0xf000000b))
    {
        fsFile gameCoin(shared, "/gamecoin.dat", FS_OPEN_READ | FS_OPEN_WRITE);

        int coinAmount = 0;
        gameCoin.seek(0x4, seek_beg);
        coinAmount = gameCoin.getByte();
        coinAmount += gameCoin.getByte() << 8;

        coinAmount = getInt("Enter a number between 0 and 300",  coinAmount, 300);
        if(coinAmount != -1)
        {
            gameCoin.seek(-2, seek_cur);
            gameCoin.putByte(coinAmount);
            gameCoin.putByte(coinAmount >> 8);
        }

        gameCoin.close();
    }
}

void saveColBin()
{
    FILE *colBin = fopen("colBin", "wb");

    fwrite(clearColor, 1, 3, colBin);
    fwrite(selColor, 1, 3, colBin);
    fwrite(unSelColor, 1, 3, colBin);

    fclose(colBin);
}

void setBGColor()
{
    showMessage("Enter RGB info for the color wanted.", "Info");

    int RGB[3];

    if( (RGB[0] = getInt("Background Red", clearColor[0], 255)) != -1 && (RGB[1] = getInt("Background Green", clearColor[1], 255)) != -1 && (RGB[2] = getInt("Background Blue", clearColor[2], 255)) != -1)
    {
        for(int i = 0; i < 3; i++)
            clearColor[i] = RGB[i];

        sf2d_set_clear_color(RGBA8(clearColor[0], clearColor[1], clearColor[2], 255));

        saveColBin();
    }
}

void setSelColor()
{
    showMessage("Enter RGB info for the color wanted.", "Info");

    int RGB[3];

    if( (RGB[0] = getInt("Selected Red", selColor[0], 255)) != -1 && (RGB[1] = getInt("Selected Green", selColor[1], 255)) != -1 && (RGB[2] = getInt("Selected Blue", selColor[2], 255)) != -1)
    {
        for(int i = 0; i < 3; i++)
            selColor[i] = RGB[i];

        saveColBin();
    }
}

void setUnselColor()
{
    showMessage("Enter RGB info for the color wanted.", "Info");

    int RGB[3];
    if( (RGB[0] = getInt("Unselected Red", unSelColor[0], 255)) != -1 && (RGB[1] = getInt("Unselected Green", unSelColor[1], 255)) != -1 && (RGB[2] = getInt("Unselected Blue", unSelColor[2], 255)) != -1)
    {
        for(int i = 0; i < 3; i++)
            unSelColor[i] = RGB[i];

        saveColBin();
    }
}

enum extraOpts
{
    setPlay,
    centeredText,
    autoBackAtRest,
    useSysLang,
    bgColor,
    slColor,
    unslColor,
    back
};

std::string onOff(bool onOff)
{
    if(onOff)
        return "On";

    return "Off";
}

void saveCfg()
{
    FILE *config = fopen("config", "wb");

    fputc(centered, config);
    fputc(autoBack, config);
    fputc(useLang, config);

    fclose(config);
}

void switchBool(bool *sw)
{
    if(*sw)
        *sw = false;
    else
        *sw = true;
}

static menu extra(136, 80, false, true);

void prepExtras()
{
    extra.addItem("Set Play Coins");
    extra.addItem(std::string("Centered text: " + onOff(centered)).c_str());
    extra.addItem(std::string("Auto Backup: " + onOff(autoBack)).c_str());
    extra.addItem(std::string("Use System Language: " + onOff(useLang)).c_str());
    extra.addItem("Set Background Color");
    extra.addItem("Set Selected Item Color");
    extra.addItem("Set Unselected Item Color");
    extra.addItem("Back");

    extra.autoVert();
}

static const std::string helpDescs[] =
{
    "Sets play coins to a number between 0 - 300",
    "Sets whether title select, nand title select, and folder select menus are centered. (Requires reboot to take effect.)",
    "Automatically creates a backup when save data is imported. Just in case!",
    "Uses system language when getting titles. Defaults to English if title is empty.",
    "Sets the background color. Asks for RGB info in that order",
    "Sets the color of selected options in menus. Asks for RGB info in that order",
    "Sets the color of unselected menu options. Asks for RGB info in that order",
    "This means go back."
};

void extrasMenu()
{
    hidScanInput();

    u32 down = hidKeysDown();

    extra.handleInput(down, 0);

    if(down & KEY_A)
    {
        switch(extra.getSelected())
        {
            case extraOpts::setPlay:
                setPlayCoins();
                break;
            case extraOpts::centeredText:
                switchBool(&centered);
                extra.updateItem(extraOpts::centeredText, std::string("Centered text: " + onOff(centered)).c_str());
                saveCfg();
                break;
            case extraOpts::autoBackAtRest:
                switchBool(&autoBack);
                extra.updateItem(extraOpts::autoBackAtRest, std::string("Auto Backup: " + onOff(autoBack)).c_str());
                saveCfg();
                break;
            case extraOpts::useSysLang:
                switchBool(&useLang);
                extra.updateItem(extraOpts::useSysLang, std::string("Use System Language: " + onOff(useLang)).c_str());
                saveCfg();
                break;
            case extraOpts::bgColor:
                setBGColor();
                break;
            case extraOpts::slColor:
                setSelColor();
                break;
            case extraOpts::unslColor:
                setUnselColor();
                break;
            case extraOpts::back:
                state = STATE_MAINMENU;
                break;
        }
    }
    else if(down & KEY_B)
        state = STATE_MAINMENU;

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
    drawTopBar(U"Config/Extras");
    extra.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        sftd_draw_text_wrap(font, 0, 0, RGBA8(255, 255, 255, 255), 12, 320, helpDescs[extra.getSelected()].c_str());
    sf2d_end_frame();

    sf2d_swapbuffers();
}

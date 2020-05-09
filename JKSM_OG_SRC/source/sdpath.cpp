#include <3ds.h>
#include <string>

#include "sdpath.h"
#include "menu.h"
#include "global.h"
#include "util.h"
#include "ui.h"
#include "file.h"

void copyListToMenu(menu *m, dirList d)
{
    m->reset();

    m->addItem("..");
    for(unsigned i = 0; i < d.count(); i++)
        m->addItem(d.retItem(i));
}

void enterDir(dirList *d, menu *m, std::u16string *path)
{
    path->append(d->retItem(m->getSelected() - 1));
    path->append((char16_t *)"/");

    d->reassign(*path);

    m->reset();

    copyListToMenu(m, *d);
}

void upDir(dirList *d, menu *m, std::u16string *path)
{
    if(path->length() < 2)
        return;

    unsigned i;
    for(i = (path->length() - 2); i > 0; i--)
    {
        if(path->c_str()[i] == L'/')
            break;
    }
    path->assign(*path, 0, i + 1);

    m->reset();

    d->reassign(*path);

    copyListToMenu(m, *d);
}

std::u16string getSDPath()
{
    //Current path on SD
    std::u16string cPath = (char16_t *)"/";

    //Menu for browsing
    menu sdBrowse(8, 16, false, false);

    //get listing and copy to sdBrowse
    dirList sdList(sdArch, cPath);
    copyListToMenu(&sdBrowse, sdList);

    button help("Help", 224, 208, 96, 32);
    std::string helpText = "Locate directory containing the save files you want to import. Press Y when finished. Press X to cancel. ";

    while(1)
    {
        hidScanInput();

        u32 down = hidKeysDown();
        u32 held = hidKeysHeld();

        sdBrowse.handleInput(down, held);

        touchPosition p;
        hidTouchRead(&p);

        if(down & KEY_A)
        {
            unsigned sel = sdBrowse.getSelected();
            if(sel == 0)
                upDir(&sdList, &sdBrowse, &cPath);
            else if(sdList.isDir(sel - 1))
                enterDir(&sdList, &sdBrowse, &cPath);
        }
        else if(down & KEY_B)
            upDir(&sdList, &sdBrowse, &cPath);
        else if(down & KEY_Y)
        {
            if(sdList.isDir(sdBrowse.getSelected() - 1) && confirm("Use this directory to restore? Everything in it will be copied to current game's save archive. "))
            {
                cPath += sdList.retItem(sdBrowse.getSelected() - 1);
                cPath += L'/';
                break;
            }
        }
        else if(down & KEY_X)
        {
            cPath = (char16_t *)"";
            break;
        }
        else if(help.released(p))
            showMessage(helpText.c_str(), "Help");

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        sftd_draw_text(font, 0, 0, RGBA8(255, 255, 255, 255), 12, toString(cPath).c_str());
        sdBrowse.draw();
        help.draw();
        sf2d_end_frame();

        sf2d_swapbuffers();
    }

    return cPath;
}

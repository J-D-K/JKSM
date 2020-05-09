#include <3ds.h>
#include <stdio.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>

#include "slot.h"
#include "titledata.h"
#include "gstr.h"
#include "menu.h"
#include "global.h"
#include "util.h"
#include "ui.h"
#include "sdpath.h"
#include "file.h"

void reinitDirMenu(menu *m, dirList *dir, std::u16string path, bool newFolder)
{
    m->reset();
    dir->reassign(path);

    for(u32 i = 0; i < dir->count(); i++)
        m->addItem(dir->retItem(i));
    if(newFolder)
        m->addItem("New");
    if(centered)
        m->centerOpts();
}

std::u16string getFolder(const titleData dat, int mode, bool newFolder)
{
    std::u16string ret;
    std::u16string path = getPath(mode) + dat.nameSafe + (char16_t)'/';

    dirList dir(sdArch, path);
    if(dir.count() == 0 && !newFolder)
    {
        showMessage("Didn't find any data to import!", "Nope");
        return ret;
    }

    menu folderMenu(40, 20, false, centered);
    for(unsigned i = 0; i < dir.count(); i++)
        folderMenu.addItem(dir.retItem(i));
    if(newFolder)
        folderMenu.addItem("New");
    if(centered)
        folderMenu.centerOpts();

    while(true)
    {
        hidScanInput();

        u32 down, held;
        down = hidKeysDown();
        held = hidKeysHeld();

        folderMenu.handleInput(down, held);

        if( (down & KEY_A) && ((u32)folderMenu.getSelected() + 1 > dir.count()))
        {
            ret = tou16(GetString("Enter a name for the new folder.").c_str());
            break;
        }
        else if(down & KEY_A)
        {
            ret = dir.retItem(folderMenu.getSelected());
            break;
        }
        else if(down & KEY_X)
        {
            std::u16string newName = tou16(GetString("Enter a new name.").c_str());
            if(!newName.empty() && (u32)folderMenu.getSelected() < dir.count())
            {
                std::u16string oldPath = path + dir.retItem(folderMenu.getSelected());
                std::u16string newPath = path + newName;

                FSUSER_RenameDirectory(sdArch, fsMakePath(PATH_UTF16, oldPath.data()), sdArch, fsMakePath(PATH_UTF16, newPath.data()));

                reinitDirMenu(&folderMenu, &dir, path, newFolder);
            }
        }
        else if(down & KEY_Y)
        {
            std::string confString = "Are you sure you want to delete '" + toString(dir.retItem(folderMenu.getSelected())) + "'?";
            if(confirm(confString.c_str()))
            {
                std::u16string delPath = path + dir.retItem(folderMenu.getSelected());

                FSUSER_DeleteDirectoryRecursively(sdArch, fsMakePath(PATH_UTF16, delPath.data()));

                reinitDirMenu(&folderMenu, &dir, path, newFolder);
                if(dir.count() == 0 && !newFolder)
                    break;
            }
        }
        else if(down & KEY_B)
            break;

        sf2d_start_frame(GFX_TOP, GFX_LEFT);
        drawTopBar(U"Select a folder. X = Rename, Y = Delete");
        folderMenu.draw();
        sf2d_end_frame();

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        sf2d_end_frame();

        sf2d_swapbuffers();
    }

    return ret;
}

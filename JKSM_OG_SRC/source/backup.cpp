#include <3ds.h>
#include <stdio.h>
#include <string>
#include <string.h>

#include "global.h"
#include "backup.h"
#include "util.h"
#include "slot.h"
#include "ui.h"
#include "date.h"
#include "file.h"
#include "titles.h"
#include "archive.h"

void copyFileToSD(FS_Archive arch, const std::u16string from, const std::u16string to)
{
    fsFile in(arch, from, FS_OPEN_READ);
    fsFile out(sdArch, to, FS_OPEN_WRITE, in.size());

    u8 *buff = new u8[buff_size];
    std::string copyString = "Copying " + toString(from) + "...";
    progressBar fileProg((float)in.size(), copyString.c_str(), "Copying file");
    u32 read, written;
    do
    {
        in.read(buff, &read, buff_size);
        out.write(buff, &written, read);
        if(written != read)
        {
            showMessage("Something went wrong writing!", "UH OH!");
            break;
        }

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        fileProg.draw((float)in.getOffset());
        sf2d_end_frame();

        sf2d_swapbuffers();
    }
    while(!in.eof());

    delete[] buff;

    in.close();
    out.close();
}

void copyDirToSD(FS_Archive save, const std::u16string from, const std::u16string to)
{
    //Use dirList to get list of dir
    dirList list(save, from);

    for(unsigned i = 0; i < list.count(); i++)
    {
        if(list.isDir(i))
        {
            std::u16string newFrom = from;
            newFrom += list.retItem(i);
            newFrom += L'/';

            std::u16string newTo = to;
            newTo += list.retItem(i);
            FSUSER_CreateDirectory(sdArch, fsMakePath(PATH_UTF16, newTo.data()), 0);
            newTo += L'/';

            copyDirToSD(save, newFrom, newTo);
        }
        else
        {
            std::u16string fullFrom = from;
            fullFrom += list.retItem(i);

            std::u16string fullTo = to;
            fullTo += list.retItem(i);

            copyFileToSD(save, fullFrom, fullTo);
        }
    }
}

bool backupData(const titleData dat, FS_Archive arch, int mode, bool autoName)
{
    //This is our path to SD folder. UTF16
    std::u16string pathOut;
    //holds name from user
    std::u16string slot;

    //if auto, just use date/time
    if(autoName)
    {
        slot = tou16(GetDate(FORMAT_YMD));
        if(autoBack)
            slot += tou16(" - AutoBack");
    }
    else
        slot = getFolder(dat, mode, true);

    if(slot.empty())
        return false;

    //get path returns path to /JKSV/[DIR]
    pathOut = getPath(mode) + dat.nameSafe + (char16_t)'/' + slot;
    std::u16string recreate = pathOut;//need this later after directory is deleted.
    pathOut += (char16_t)'/';

    //I only do this because games use more files for more slots.
    FSUSER_DeleteDirectoryRecursively(sdArch, fsMakePath(PATH_UTF16, pathOut.data()));
    //recreate it.
    FSUSER_CreateDirectory(sdArch, fsMakePath(PATH_UTF16, recreate.data()), 0);

    //archive root
    std::u16string pathIn = (char16_t *)"/";

    copyDirToSD(arch, pathIn, pathOut);

    //This gets annoying in auto mode
    if(!autoName)
        showMessage("Finished!", "Success!");

    return true;
}

void autoBackup(menu m)
{
    showMessage("This can take a few minutes depending on how many titles are selected.", "Info");


    progressBar autoDump((float)m.getSelectCount(), "Copying saves... ", "Auto Backup");
    //Keep track of what's done
    float dumpCount = 0;
    for(unsigned i = 0; i < m.getSize(); i++)
    {
        //This is for titles with no save archive ex. Fantasy Life
        bool dumped = false;
        FS_Archive saveArch;
        if(m.optSelected(i) && openSaveArch(&saveArch, sdTitle[i], false))   //if it's selected and we can open save archive
        {
            createTitleDir(sdTitle[i], MODE_SAVE);
            backupData(sdTitle[i], saveArch, MODE_SAVE, true);
            dumpCount++;
            dumped = true;
        }
        FSUSER_CloseArchive(saveArch);

        FS_Archive extArch;
        if(m.optSelected(i) && openExtdata(&extArch, sdTitle[i], false))
        {
            createTitleDir(sdTitle[i], MODE_EXTDATA);
            backupData(sdTitle[i], extArch, MODE_EXTDATA, true);

            //check first to make sure we don't count it twice because no save arch
            if(!dumped)
                dumpCount++;
        }
        FSUSER_CloseArchive(extArch);

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        autoDump.draw(i);
        sf2d_end_frame();
        sf2d_swapbuffers();
    }

    showMessage("Finished!", "Success!");
}

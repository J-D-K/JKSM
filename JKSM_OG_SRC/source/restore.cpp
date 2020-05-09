#include <3ds.h>
#include <string>
#include <sf2d.h>
#include <sftd.h>

#include "restore.h"
#include "backup.h"
#include "archive.h"
#include "slot.h"
#include "global.h"
#include "util.h"
#include "ui.h"
#include "sdpath.h"
#include "file.h"
#include "titles.h"
#include "archive.h"

void copyFiletoArch(FS_Archive arch, const std::u16string from, const std::u16string to, int mode)
{
    fsFile in(sdArch, from, FS_OPEN_READ);
    fsFile out(arch, to, FS_OPEN_WRITE, in.size());

    u8 *buff = new u8[buff_size];
    std::string copyString = "Copying " + toString(from) + "...";
    progressBar fileProg((float)in.size(), copyString.c_str(), "Copying File");
    u32 read, written;
    do
    {
        in.read(buff, &read, buff_size);
        out.write(buff, &written, read);
        if(written != read)
        {
            showMessage("Something went wrong writing to the archive file!", "UH OH!");
            break;
        }

        sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
        fileProg.draw(in.getOffset());
        sf2d_end_frame();

        sf2d_swapbuffers();
    }
    while(!in.eof());

    delete[] buff;

    in.close();
    out.close();
}

void copyDirToArch(FS_Archive arch, const std::u16string from, const std::u16string to, int mode)
{
    dirList list(sdArch, from);
    for(unsigned i = 0; i < list.count(); i++)
    {
        if(list.isDir(i))
        {
            std::u16string newFrom = from + list.retItem(i) + (char16_t)'/';

            std::u16string newTo = to + list.retItem(i);
            FSUSER_CreateDirectory(arch, fsMakePath(PATH_UTF16, newTo.data()), 0);
            newTo += L'/';

            copyDirToArch(arch, newFrom, newTo, mode);
        }
        else
        {
            std::u16string sdPath = from + list.retItem(i);

            std::u16string archPath = to + list.retItem(i);

            copyFiletoArch(arch, sdPath, archPath, mode);
        }
    }
}

bool restoreData(const titleData dat, FS_Archive arch, int mode)
{
    std::u16string sdPath;

    std::u16string keepName = getFolder(dat, mode, false);
    if(keepName.empty())
        return false;

    std::string ask = "Are you sure you want to import " + toString(keepName) + "?";
    if(!confirm(ask.c_str()))
        return false;

    if(autoBack)
        backupData(dat, arch, mode, true);

    sdPath = getPath(mode) + dat.nameSafe + (char16_t)'/' + keepName + (char16_t)'/';

    std::u16string archPath = (char16_t *)"/";

    if(!modeExtdata(mode))
        FSUSER_DeleteDirectoryRecursively(arch, fsMakePath(PATH_ASCII, "/"));


    copyDirToArch(arch, sdPath, archPath, mode);

    //If we're not restoring some kind of extdata, commit save data
    if(!modeExtdata(mode))
    {
        fsCommitData(arch);
        //If we're running under something from the hbl, end the session, delete the SV and start it again.
        if(hbl)
            fsEnd();

        deleteSV(dat);

        if(hbl)
            fsStart();
    }


    showMessage("Finished", "Success!");

    return true;
}

bool restoreDataSDPath(const titleData dat, FS_Archive arch, int mode)
{
    std::u16string sdPath = getSDPath();
    if(sdPath.length() < 2)
        return false;

    std::u16string archPath = (char16_t *)"/";

    if(!modeExtdata(mode))
        FSUSER_DeleteDirectoryRecursively(arch, fsMakePath(PATH_ASCII, "/"));

    if(autoBack)
        backupData(dat, arch, mode, true);

    copyDirToArch(arch, sdPath, archPath, mode);

    if(!modeExtdata(mode))
    {
        fsCommitData(arch);

        if(hbl)
            fsEnd();

        deleteSV(dat);

        if(hbl)
            fsStart();
    }

    showMessage("Finished!", "Success!");

    return true;
}

void autoRestore(menu m)
{
    //This still needs user input.
    for(unsigned i = 0; i < m.getSize(); i++)
    {
        FS_Archive saveArch;
        if(m.optSelected(i) && openSaveArch(&saveArch, sdTitle[i], false))
            restoreData(sdTitle[i], saveArch, MODE_SAVE);
        FSUSER_CloseArchive(saveArch);

        FS_Archive extArch;
        if(m.optSelected(i) && openExtdata(&extArch, sdTitle[i], false))
            restoreData(sdTitle[i], extArch, MODE_EXTDATA);
        FSUSER_CloseArchive(extArch);
    }
}

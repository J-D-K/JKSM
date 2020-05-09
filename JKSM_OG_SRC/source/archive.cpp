#include <3ds.h>

#include "titledata.h"
#include "archive.h"
#include "ui.h"

//All the information used here was found on 3dbrew.org
//thank them too.
bool openSaveArch(FS_Archive *out, const titleData dat, bool show)
{
    //binary path
    u32 path[3] = {dat.media, dat.low, dat.high};

    FS_Path binPath = {PATH_BINARY, 12, path};

    Result res = FSUSER_OpenArchive(out, ARCHIVE_USER_SAVEDATA, binPath);
    if(res)
    {
        if(show)
            showError("Error opening save archive", (unsigned)res);
        return false;
    }

    return true;
}

bool openSaveArch3dsx(FS_Archive *arch)
{
    Result res = FSUSER_OpenArchive(arch, ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""));
    if(res)
    {
        showError("Error opening save archive", (unsigned)res);
        return false;
    }

    return true;
}

bool openExtdata(FS_Archive *out, const titleData dat, bool show)
{
    u32 path[3] = {MEDIATYPE_SD, dat.extdata, 0};

    FS_Path binPath = {PATH_BINARY, 12, path};

    Result res = FSUSER_OpenArchive(out, ARCHIVE_EXTDATA, binPath);
    if(res)
    {
        if(show)
            showError("Error opening ExtData", (unsigned)res);
        return false;
    }

    return true;
}

bool openSharedExt(FS_Archive *out, u32 id)
{
    u32 path[3] = {MEDIATYPE_NAND, id, 0x00048000};

    FS_Path binPath = {PATH_BINARY, 12, path};

    Result res = FSUSER_OpenArchive(out, ARCHIVE_SHARED_EXTDATA, binPath);
    if(res)
    {
        showError("Error opening Shared Extdata", (unsigned)res);
        return false;
    }

    return true;
}

bool openBossExt(FS_Archive *out, const titleData dat)
{
    u32 path[3] = {MEDIATYPE_SD, dat.extdata, 0};

    FS_Path binPath = {PATH_BINARY, 12, path};

    Result res = FSUSER_OpenArchive(out, ARCHIVE_BOSS_EXTDATA, binPath);
    if(res)
    {
        showError("Error opening Boss Extdata", (unsigned)res);
        return false;
    }

    return true;
}

bool openSysModule(FS_Archive *out, const titleData dat)
{
    u32 path[2] = {MEDIATYPE_NAND, (0x00010000 | dat.unique)};

    FS_Path binPath = {PATH_BINARY, 8, path};

    Result res = FSUSER_OpenArchive(out, ARCHIVE_SYSTEM_SAVEDATA, binPath);
    if(res)
    {
        return false;
    }

    return true;
}

bool openSysSave(FS_Archive *out, const titleData dat)
{
    u32 path[2] = {MEDIATYPE_NAND, (0x00020000 | dat.unique)};

    FS_Path binPath = {PATH_BINARY, 8, path};

    Result res = FSUSER_OpenArchive(out, ARCHIVE_SYSTEM_SAVEDATA, binPath);
    if(res)
    {
        //try opening as module instead
        if(openSysModule(out, dat))
            return true;
        else
        {
            showError("Error opening system save data", (unsigned)res);
            return false;
        }
    }

    return true;
}

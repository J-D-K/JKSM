#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "smdh.h"
#include "global.h"
#include "file.h"
#include "ui.h"

//Pretty much stolen from the hbl
/*smdh_s *loadSMDH(u32 Low, u32 High, u8 Media)
{
    smdh_s *ret = new smdh_s;
    u32 archPath[] = {Low, High, Media, 0x0};
    static const u32 filePath[] = { 0x0, 0x0, 0x2, 0x6E6F6369, 0x0};

    FS_Path binArchPath = {PATH_BINARY, 0x10, archPath};
    FS_Path binFilePath = {PATH_BINARY, 0x14, filePath};

    fsFile smdh(ARCHIVE_SAVEDATA_AND_CONTENT, binArchPath, binFilePath, FS_OPEN_READ);
    if(smdh.isOpened() && (smdh.read(ret, NULL, sizeof(smdh_s)) == 0))
    {
        smdh.close();
        return ret;
    }
    else
    {
        smdh.close();
        delete ret;
        return NULL;
    }
}*/

smdh_s *loadSMDH(u32 Low, u32 High, u8 Media)
{
    //Pretty much stolen from hb_menu. It's the only thing I could find with how to open this.
    Handle FileHandle;

    u32 archPath[] = {Low, High, Media, 0x0};
    static const u32 filePath[] = { 0x0, 0x0, 0x2, 0x6E6F6369, 0x0};
    smdh_s *Ret = new smdh_s;

    FS_Path binArchPath = {PATH_BINARY, 0x10, archPath};
    FS_Path binFilePath = {PATH_BINARY, 0x14, filePath};

    Result Res = FSUSER_OpenFileDirectly(&FileHandle, ARCHIVE_SAVEDATA_AND_CONTENT, binArchPath, binFilePath, FS_OPEN_READ, 0);
    if(Res == 0)
    {
        //For bytes read.
        u32 Read;

        //Read it into Out
        FSFILE_Read(FileHandle, &Read, 0, Ret, sizeof(smdh_s));
    }
    else
    {
        delete Ret;
        Ret = NULL;
    }

    FSFILE_Close(FileHandle);
    return Ret;
}

#include <3ds.h>
#include <string>
#include <vector>

#include "ui.h"
#include "fs.h"
#include "util.h"
#include "type.h"
#include "cfg.h"

static ui::menu fldMenu;
static fs::dirList fldList;
static std::u16string targetDir;
static std::string uploadParent;
std::vector<drive::gdItem *> gdList;

void fldMenuNew(void *a)
{
    std::u16string newFolder;
    uint32_t held = ui::padKeysHeld();

    if(held & KEY_L)
        newFolder = util::toUtf16(util::getDateString(util::DATE_FMT_YDM));
    else if(held & KEY_R)
        newFolder = util::toUtf16(util::getDateString(util::DATE_FMT_YMD));
    else
        newFolder = util::safeString(util::toUtf16(util::getString("Enter a new folder name", true)));
    
    if(!newFolder.empty() && cfg::config["zip"])
    {
        std::u16string fullOut = targetDir + newFolder + util::toUtf16(".zip");
        fs::copyArchToZipThreaded(fs::getSaveArch(), util::toUtf16("/"), fullOut);
    }
    else if(!newFolder.empty())
    {
        std::u16string fullOut = targetDir + newFolder;
        std::u16string svRoot  = util::toUtf16("/");

        FS_Path crDir = fsMakePath(PATH_UTF16, fullOut.c_str());
        FSUSER_CreateDirectory(fs::getSDMCArch(), crDir, 0);
        fullOut += util::toUtf16("/");

        fs::copyDirToDirThreaded(fs::getSaveArch(), util::toUtf16("/"), fs::getSDMCArch(), fullOut, false);
    }
}

void fldMenuOverwrite_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    fs::dirItem *in = (fs::dirItem *)t->argPtr;
    t->status->setStatus("Overwriting backup...");
    if(in->isDir)
    {
        std::u16string overwrite = targetDir + in->name;
        FS_Path delPath = fsMakePath(PATH_UTF16, overwrite.c_str());
        FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), delPath);
        FSUSER_CreateDirectory(fs::getSDMCArch(), delPath, 0);
        overwrite += util::toUtf16("/");
        fs::copyDirToDir(fs::getSaveArch(), util::toUtf16("/"), fs::getSDMCArch(), overwrite, false, NULL);
    }
    else
    {
        std::u16string overwrite = targetDir + in->name;
        FS_Path targetPath = fsMakePath(PATH_UTF16, overwrite.c_str());
        FS_Path tmpPath = fsMakePath(PATH_ASCII, "/JKSV/tmp.zip");
        
        FSUSER_DeleteFile(fs::getSDMCArch(), targetPath);

        zipFile zip = zipOpen64("/JKSV/tmp.zip", 0);
        fs::copyArchToZip(fs::getSaveArch(), util::toUtf16("/"), zip, NULL);
        zipClose(zip, NULL);

        FSUSER_RenameFile(fs::getSDMCArch(), tmpPath, fs::getSDMCArch(), targetPath);
    }
    t->finished = true;
}

void fldMenuOverwrite(void *a)
{
    fs::dirItem *in = (fs::dirItem *)a;
    std::string q = "Are you sure you'd like to overwrite " + util::toUtf8(in->name) + "?";
    ui::confirm(q, fldMenuOverwrite_t, NULL, a);
}

void fldMenuDelete_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    fs::dirItem *in = (fs::dirItem *)t->argPtr;
    t->status->setStatus("Deleting backup...");
    std::u16string del = targetDir + in->name;
    if(in->isDir)
        fs::delDirRec(fs::getSDMCArch(), del);
    else
        FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_UTF16, del.c_str()));

    ui::fldRefresh();

    t->finished = true;
}

void fldMenuDelete(void *a)
{
    fs::dirItem *in = (fs::dirItem *)a;
    std::string q = "Are you sure you would like to delete " + util::toUtf8(in->name) + "?";
    ui::confirm(q, fldMenuDelete_t, NULL, a);
}

void fldMenuRestore_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    fs::dirItem *in = (fs::dirItem *)t->argPtr;
    fs::deleteSv(fs::getSaveMode());
    if(in->isDir)
    {
        std::u16string rest = targetDir + in->name + util::toUtf16("/");
        fs::delDirRec(fs::getSaveArch(), util::toUtf16("/"));
        fs::commitData(fs::getSaveMode());
        t->status->setStatus("Restoring data to archive...");
        fs::copyDirToDir(fs::getSDMCArch(), rest, fs::getSaveArch(), util::toUtf16("/"), true, NULL);
    }
    else
    {
        std::u16string rest = targetDir + in->name;
        FS_Path srcPath = fsMakePath(PATH_UTF16, rest.c_str());
        FS_Path tmpPath = fsMakePath(PATH_ASCII, "/JKSV/tmp.zip");
        FSUSER_RenameFile(fs::getSDMCArch(), srcPath, fs::getSDMCArch(), tmpPath);

        fs::delDirRec(fs::getSaveArch(), util::toUtf16("/"));
        fs::commitData(fs::getSaveMode());

        t->status->setStatus("Decompressing zip to archive...");
        unzFile unz = unzOpen64("/JKSV/tmp.zip");
        fs::copyZipToArch(fs::getSaveArch(), unz, t);
        unzClose(unz);

        FSUSER_RenameFile(fs::getSDMCArch(), tmpPath, fs::getSDMCArch(), srcPath);
    }
    t->finished = true;
}

//Creates confirm that jumps to ^
void fldMenuRestore(void *a)
{
    fs::dirItem *in = (fs::dirItem *)a;
    std::string q = "Are you sure you want to restore " + util::toUtf8(in->name) + "?";
    ui::confirm(q, fldMenuRestore_t, NULL, a);
}

void fldMenuUpload_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    fs::dirItem *in = (fs::dirItem *)t->argPtr;
    std::u16string src = targetDir + in->name;
    t->status->setStatus("Uploading " + util::toUtf8(in->name) + "...");

    //For now
    FS_Path srcPath = fsMakePath(PATH_UTF16, src.c_str());
    FS_Path tmpPath = fsMakePath(PATH_ASCII, "/JKSV/tmp.zip");
    FSUSER_RenameFile(fs::getSDMCArch(), srcPath, fs::getSDMCArch(), tmpPath);

    FILE *upload = fopen("/JKSV/tmp.zip", "rb");
    std::string utf8Name = util::toUtf8(in->name);
    if(fs::gDrive->fileExists(utf8Name, uploadParent))
    {
        std::string fileID = fs::gDrive->getFileID(utf8Name, uploadParent);
        fs::gDrive->updateFile(fileID, upload);
    }
    else
        fs::gDrive->uploadFile(utf8Name, uploadParent, upload);

    fclose(upload);

    FSUSER_RenameFile(fs::getSDMCArch(), tmpPath, fs::getSDMCArch(), srcPath);

    ui::fldRefresh();

    t->finished = true;
}

void fldMenuUpload(void *a)
{
    fs::dirItem *in = (fs::dirItem *)a;
    if(fs::gDrive && !in->isDir)
        ui::newThread(fldMenuUpload_t, a, NULL);
}

void fldMenuDriveDownload_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    drive::gdItem *in = (drive::gdItem *)t->argPtr;

    t->status->setStatus("Downloading " + in->name + "...");

    std::u16string target = targetDir + util::toUtf16(in->name);
    FS_Path targetPath = fsMakePath(PATH_UTF16, target.c_str());
    FS_Path tmpPath = fsMakePath(PATH_ASCII, "/JKSV/tmp.zip");

    if(fs::fsfexists(fs::getSDMCArch(), target))
        FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_UTF16, target.c_str()));

    FILE *tmp = fopen("/JKSV/tmp.zip", "wb");
    fs::gDrive->downloadFile(in->id, tmp);
    fclose(tmp);

    FSUSER_RenameFile(fs::getSDMCArch(), tmpPath, fs::getSDMCArch(), targetPath);

    ui::fldRefresh();

    t->finished = true;
}

void fldMenuDriveDownload(void *a)
{
    drive::gdItem *in = (drive::gdItem *)a;
    std::u16string checkPath = targetDir + util::toUtf16(in->name);
    if(fs::fsfexists(fs::getSDMCArch(), checkPath))
        ui::confirm("Downloading this backup will replace the one on your SD card. Are you sure you still want to download it?", fldMenuDriveDownload_t, NULL, a);
    else
        ui::newThread(fldMenuDriveDownload_t, a, NULL);
}

void fldMenuDriveDelete_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    drive::gdItem *in = (drive::gdItem *)t->argPtr;
    t->status->setStatus("Deleting " + in->name + "...");
    fs::gDrive->deleteFile(in->id);
    ui::fldRefresh();
    t->finished = true;
}

void fldMenuDriveDelete(void *a)
{
    drive::gdItem *in = (drive::gdItem *)a;
    ui::confirm("Are you sure you want to delete " + in->name + " from your drive?", fldMenuDriveDelete_t, NULL, a);
}

void fldMenuDriveRestore_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    drive::gdItem *in = (drive::gdItem *)t->argPtr;
    t->status->setStatus("Downloading " + in->name + "...");

    FILE *tmp = fopen("/JKSV/tmp.zip", "wb");
    fs::gDrive->downloadFile(in->id, tmp);
    fclose(tmp);

    t->status->setStatus("Decompressing backup to archive...");
    unzFile unz = unzOpen64("/JKSV/tmp.zip");
    fs::copyZipToArch(fs::getSaveArch(), unz, NULL);
    unzClose(unz);

    FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, "/JKSV/tmp.zip"));

    t->finished = true;
}

void fldMenuDriveRestore(void *a)
{
    drive::gdItem *in = (drive::gdItem *)a;
    ui::confirm("Are you sure you want to download and restore " + in->name + "?", fldMenuDriveRestore_t, NULL, a);
}

void ui::fldInit(const std::u16string& _path, const std::string& _uploadParent, funcPtr _func, void *_args)
{
    fldMenu.reset();
    fldMenu.setCallback(_func, _args);
    fldList.reassign(fs::getSDMCArch(), _path);
    targetDir = _path;
    uploadParent = _uploadParent;

    fldMenu.addOpt("New", 0);
    fldMenu.addOptEvent(0, KEY_A, fldMenuNew, NULL);

    int fldInd = 1;
    if(fs::gDrive)
    {
        fs::gDrive->getListWithParent(uploadParent, gdList);

        for(unsigned i = 0; i < gdList.size(); i++, fldInd++)
        {
            fldMenu.addOpt("[GD] " + gdList[i]->name, 320);

            fldMenu.addOptEvent(fldInd, KEY_A, fldMenuDriveDownload, gdList[i]);
            fldMenu.addOptEvent(fldInd, KEY_X, fldMenuDriveDelete, gdList[i]);
            fldMenu.addOptEvent(fldInd, KEY_Y, fldMenuDriveRestore, gdList[i]);
        }
    }

    for(unsigned i = 0; i < fldList.getCount(); i++, fldInd++)
    {
        fldMenu.addOpt(util::toUtf8(fldList.getItem(i)), 320);

        fs::dirItem *di = fldList.getDirItemAt(i);
        fldMenu.addOptEvent(fldInd, KEY_A, fldMenuOverwrite, di);
        fldMenu.addOptEvent(fldInd, KEY_X, fldMenuDelete, di);
        fldMenu.addOptEvent(fldInd, KEY_Y, fldMenuRestore, di);
        fldMenu.addOptEvent(fldInd, KEY_R, fldMenuUpload, di);
    }
}

void ui::fldRefresh()
{
    fldMenu.reset();
    fldList.reassign(fs::getSDMCArch(), targetDir);

    fldMenu.addOpt("New", 0);
    fldMenu.addOptEvent(0, KEY_A, fldMenuNew, NULL);

    int fldInd = 1;
    if(fs::gDrive)
    {
        fs::gDrive->getListWithParent(uploadParent, gdList);

        for(unsigned i = 0; i < gdList.size(); i++, fldInd++)
        {
            fldMenu.addOpt("[GD] " + gdList[i]->name, 320);

            fldMenu.addOptEvent(fldInd, KEY_A, fldMenuDriveDownload, gdList[i]);
            fldMenu.addOptEvent(fldInd, KEY_X, fldMenuDriveDelete, gdList[i]);
            fldMenu.addOptEvent(fldInd, KEY_Y, fldMenuDriveRestore, gdList[i]);
        }
    }

    for(unsigned i = 0; i < fldList.getCount(); i++, fldInd++)
    {
        fldMenu.addOpt(util::toUtf8(fldList.getItem(i)), 320);

        fs::dirItem *di = fldList.getDirItemAt(i);
        fldMenu.addOptEvent(fldInd, KEY_A, fldMenuOverwrite, di);
        fldMenu.addOptEvent(fldInd, KEY_X, fldMenuDelete, di);
        fldMenu.addOptEvent(fldInd, KEY_Y, fldMenuRestore, di);
        fldMenu.addOptEvent(fldInd, KEY_R, fldMenuUpload, di);
    }
}

void ui::fldUpdate()
{
    fldMenu.update();
}

void ui::fldDraw()
{
    fldMenu.draw(0, 2, 0xFFFFFFFF, 320, false);
}

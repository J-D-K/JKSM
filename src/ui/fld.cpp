#include <3ds.h>
#include <string>

#include "ui.h"
#include "fs.h"
#include "util.h"
#include "type.h"
#include "cfg.h"

static ui::menu fldMenu;
static fs::dirList fldList;
static std::u16string targetDir;

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
    int sel = fldMenu.getSelected() - 1;
    t->status->setStatus("Overwriting backup...");
    if(fldList.isDir(sel))
    {
        std::u16string overwrite = targetDir + fldList.getItem(sel);
        FS_Path delPath = fsMakePath(PATH_UTF16, overwrite.c_str());
        FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), delPath);
        FSUSER_CreateDirectory(fs::getSDMCArch(), delPath, 0);
        overwrite += util::toUtf16("/");
        fs::copyDirToDir(fs::getSaveArch(), util::toUtf16("/"), fs::getSDMCArch(), overwrite, false, NULL);
    }
    else
    {
        std::u16string overwrite = targetDir + fldList.getItem(sel);
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
    int sel = fldMenu.getSelected() - 1;
    std::string q = "Are you sure you'd like to overwrite " + util::toUtf8(fldList.getItem(sel)) + "?";
    ui::confirm(q, fldMenuOverwrite_t, NULL, NULL);
}

void fldMenuDelete(void *a)
{
    int sel = fldMenu.getSelected() - 1;
    std::u16string del = targetDir + fldList.getItem(sel);
    if(fldList.isDir(sel))
        fs::delDirRec(fs::getSDMCArch(), del);
    else
        FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_UTF16, del.c_str()));

    ui::fldRefresh();
}

void fldMenuRestore_t(void *a)
{
    threadInfo *t = (threadInfo *)a;
    fs::deleteSv(fs::getSaveMode());
    int sel = fldMenu.getSelected() - 1;
    if(fldList.isDir(sel))
    {
        std::u16string rest = targetDir + fldList.getItem(sel) + util::toUtf16("/");
        fs::delDirRec(fs::getSaveArch(), util::toUtf16("/"));
        fs::commitData(fs::getSaveMode());
        t->status->setStatus("Restoring data to archive...");
        fs::copyDirToDir(fs::getSDMCArch(), rest, fs::getSaveArch(), util::toUtf16("/"), true, NULL);
    }
    else
    {
        std::u16string rest = targetDir + fldList.getItem(sel);
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
    int sel = fldMenu.getSelected() - 1;
    std::string q = "Are you sure you want to restore " + util::toUtf8(fldList.getItem(sel)) + "?";
    ui::confirm(q, fldMenuRestore_t, NULL, NULL);
}

void ui::fldInit(const std::u16string& _path, funcPtr _func, void *_args)
{
    fldMenu.reset();
    fldMenu.setCallback(_func, _args);
    fldList.reassign(fs::getSDMCArch(), _path);
    targetDir = _path;

    fldMenu.addOpt("New", 0);
    fldMenu.addOptEvent(0, KEY_A, fldMenuNew, NULL);
    for(unsigned i = 0; i < fldList.getCount(); i++)
    {
        fldMenu.addOpt(util::toUtf8(fldList.getItem(i)), 320);

        fldMenu.addOptEvent(i + 1, KEY_A, fldMenuOverwrite, NULL);
        fldMenu.addOptEvent(i + 1, KEY_X, fldMenuDelete, NULL);
        fldMenu.addOptEvent(i + 1, KEY_Y, fldMenuRestore, NULL);
    }
}

void ui::fldRefresh()
{
    fldMenu.reset();
    fldList.reassign(fs::getSDMCArch(), targetDir);

    fldMenu.addOpt("New", 0);
    fldMenu.addOptEvent(0, KEY_A, fldMenuNew, NULL);
    for(unsigned i = 0; i < fldList.getCount(); i++)
    {
        fldMenu.addOpt(util::toUtf8(fldList.getItem(i)), 320);

        fldMenu.addOptEvent(i + 1, KEY_A, fldMenuOverwrite, NULL);
        fldMenu.addOptEvent(i + 1, KEY_X, fldMenuDelete, NULL);
        fldMenu.addOptEvent(i + 1, KEY_Y, fldMenuRestore, NULL);
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

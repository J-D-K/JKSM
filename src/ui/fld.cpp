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
        zipFile zip = zipOpen64("/tmp.zip", 0);
        std::u16string fullOut = targetDir + newFolder + util::toUtf16(".zip");
        fs::copyArchToZip(fs::getSaveArch(), util::toUtf16("/"), zip);
        zipClose(zip, NULL);
        FSUSER_RenameFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, "/tmp.zip"), fs::getSDMCArch(), fsMakePath(PATH_UTF16, fullOut.c_str()));  
    }
    else if(!newFolder.empty())
    {
        std::u16string fullOut = targetDir + newFolder;
        std::u16string svRoot  = util::toUtf16("/");

        FS_Path crDir = fsMakePath(PATH_UTF16, fullOut.c_str());
        FSUSER_CreateDirectory(fs::getSDMCArch(), crDir, 0);
        fullOut += util::toUtf16("/");

        fs::copyDirToSD(fs::getSaveArch(), svRoot, fullOut);
    }
    ui::fldRefresh();
}

void fldMenuOverwrite(void *a)
{
    int sel = fldMenu.getSelected() - 1;
    if(fldList.isDir(sel))
    {
        std::u16string overwrite = targetDir + fldList.getItem(sel);
        FS_Path delPath = fsMakePath(PATH_UTF16, overwrite.c_str());
        FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), delPath);
        FSUSER_CreateDirectory(fs::getSDMCArch(), delPath, 0);
        overwrite += util::toUtf16("/");
        fs::copyDirToSD(fs::getSaveArch(), util::toUtf16("/"), overwrite);
    }
    else
    {
        std::u16string overwrite = targetDir + fldList.getItem(sel);
        FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_UTF16, overwrite.c_str()));
        zipFile zip = zipOpen64("/tmp.zip", 0);
        fs::copyArchToZip(fs::getSaveArch(), util::toUtf16("/"), zip);
        zipClose(zip, NULL);
        FSUSER_RenameFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, "/tmp.zip"), fs::getSDMCArch(), fsMakePath(PATH_UTF16, overwrite.c_str()));
    }
    
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

void fldMenuRestore(void *a)
{
    fs::deleteSv(fs::getSaveMode());
    int sel = fldMenu.getSelected() - 1;
    if(fldList.isDir(sel))
    {
        std::u16string rest = targetDir + fldList.getItem(sel) + util::toUtf16("/");
        fs::delDirRec(fs::getSaveArch(), util::toUtf16("/"));
        fs::commitData(fs::getSaveMode());
        fs::copyDirToArch(fs::getSaveArch(), rest, util::toUtf16("/"));
    }
    else
    {
        std::u16string zipTarget = targetDir + fldList.getItem(sel);
        FSUSER_RenameFile(fs::getSDMCArch(), fsMakePath(PATH_UTF16, zipTarget.c_str()), fs::getSDMCArch(), fsMakePath(PATH_ASCII, "/tmp.zip"));
        fs::delDirRec(fs::getSaveArch(), util::toUtf16("/"));
        fs::commitData(fs::getSaveArch());
        unzFile unz = unzOpen64("/tmp.zip");
        fs::copyZipToArch(fs::getSaveArch(), unz);
        unzClose(unz);
        FSUSER_RenameFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, "/tmp.zip"), fs::getSDMCArch(), fsMakePath(PATH_UTF16, zipTarget.c_str()));
    }
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

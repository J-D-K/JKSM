#include <3ds.h>
#include <string>

#include "ui.h"
#include "fs.h"
#include "util.h"
#include "type.h"

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

    if(!newFolder.empty())
    {
        std::u16string fullOut = targetDir + newFolder;
        std::u16string svRoot  = util::toUtf16("/");

        FS_Path crDir = fsMakePath(PATH_UTF16, fullOut.c_str());
        FSUSER_CreateDirectory(fs::getSDMCArch(), crDir, 0);
        fullOut += util::toUtf16("/");

        fs::copyDirToSD(fs::getSaveArch(), svRoot, fullOut);
    }
}

//To do: this + restore using threaded funcs
void fldMenuOverwrite(void *a)
{
    //Skip new
    std::u16string overwrite = targetDir + fldList.getItem(fldMenu.getSelected() - 1);
    std::u16string svRoot    = util::toUtf16("/");
    FS_Path delPath = fsMakePath(PATH_UTF16, overwrite.c_str());
    FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), delPath);
    FSUSER_CreateDirectory(fs::getSDMCArch(), delPath, 0);
    overwrite += util::toUtf16("/");
    fs::copyDirToSD(fs::getSaveArch(), svRoot, overwrite);
}

void fldMenuDelete(void *a)
{
    std::u16string del = targetDir + fldList.getItem(fldMenu.getSelected() - 1);
    fs::delDirRec(fs::getSDMCArch(), del);
    ui::newThread(ui::fldRefresh, NULL, NULL);
}

void fldMenuRestore(void *a)
{
    fs::deleteSv(fs::getSaveMode());
    std::u16string rest = targetDir + fldList.getItem(fldMenu.getSelected() - 1) + util::toUtf16("/");
    std::u16string svRoot = util::toUtf16("/");
    FS_Path delRoot = fsMakePath(PATH_UTF16, svRoot.c_str());
    FSUSER_DeleteDirectoryRecursively(fs::getSaveArch(), delRoot);
    fs::commitData(fs::getSaveMode());
    fs::copyDirToArch(fs::getSaveArch(), rest, svRoot);
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

void ui::fldRefresh(void *a)
{
    threadInfo *t = NULL;
    if(a)
        t = (threadInfo *)a;

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
    if(t)
        t->finished = true;
}

void ui::fldUpdate()
{
    fldMenu.update();
}

void ui::fldDraw()
{
    fldMenu.draw(0, 2, 0xFFFFFFFF, 320, false);
}

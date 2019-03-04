#include <string>
#include <cstring>
#include <3ds.h>

#include "ui.h"
#include "gfx.h"
#include "fs.h"
#include "util.h"

static ui::menu saveMenu, sdMenu, copyMenu;
static fs::dirList saveList(fs::getSaveArch(), util::toUtf16("")), sdList(fs::getSDMCArch(), util::toUtf16(""));
static std::string savePathDisp, sdPathDisp;
static std::u16string savePath, sdPath;

static int advMenuCtrl = 0, advPrev = 0;

//Program state
extern int state;

enum
{
    SD_TO_ARCH,
    ARCH_TO_SD
};

bool confirmTransfer(const std::u16string& from, const std::u16string& to, int way)
{
    std::string fromDrive, toDrive;
    switch(way)
    {
        case SD_TO_ARCH:
            fromDrive = "sd:";
            toDrive   = "sv:";
            break;

        case ARCH_TO_SD:
            fromDrive = "sv:";
            toDrive   = "sd:";
            break;
    }

    std::string conf = "Are you sure you want to copy '" + fromDrive + util::toUtf8(from) + "' to '" + toDrive + util::toUtf8(to) + "'?";

    return ui::confirm(conf);
}

bool confirmDelete(const std::u16string& del, int way)
{
    std::string drive;
    switch(way)
    {
        case SD_TO_ARCH:
            drive = "sd:";
            break;

        case ARCH_TO_SD:
            drive = "sv:";
            break;
    }

    std::string conf = "Are you sure you want to delete '" + drive + util::toUtf8(del) + "'?";
    return ui::confirm(conf);
}

void performCopyOps()
{
    switch(copyMenu.getSelected())
    {
        //Copy From
        case 0:
            {
                switch(advPrev)
                {
                    case 0://Save was active
                        {
                            if(saveMenu.getSelected() == 0)
                            {
                                if(confirmTransfer(savePath, sdPath, ARCH_TO_SD))
                                    fs::copyDirToSD(fs::getSaveArch(), savePath, sdPath);
                            }
                            else if(saveMenu.getSelected() > 1)
                            {
                                int saveSel = saveMenu.getSelected() - 2;
                                if(saveList.isDir(saveSel))
                                {
                                    std::u16string fromPath = savePath + saveList.getItem(saveSel) + util::toUtf16("/");
                                    std::u16string toPath = sdPath + saveList.getItem(saveSel);
                                    if(confirmTransfer(fromPath, toPath, ARCH_TO_SD))
                                    {
                                        FSUSER_CreateDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, toPath.data()), 0);
                                        toPath += util::toUtf16("/");

                                        fs::copyDirToSD(fs::getSaveArch(), fromPath, toPath);
                                    }
                                }
                                else
                                {
                                    std::u16string fromPath = savePath + saveList.getItem(saveSel);
                                    std::u16string toPath = sdPath + saveList.getItem(saveSel);
                                    if(confirmTransfer(fromPath, toPath, ARCH_TO_SD))
                                        fs::copyFileToSD(fs::getSaveArch(), fromPath, toPath);
                                }
                            }
                        }
                        break;

                    case 1://SD was active
                        {
                            if(sdMenu.getSelected() == 0)
                            {
                                if(confirmTransfer(sdPath, savePath, SD_TO_ARCH))
                                {
                                    fs::copyDirToArch(fs::getSaveArch(), sdPath, savePath);
                                    fs::commitData(fs::getSaveMode());
                                }
                            }
                            else if(sdMenu.getSelected() > 1)
                            {
                                int sdSel = sdMenu.getSelected() - 2;
                                if(sdList.isDir(sdSel))
                                {
                                    std::u16string fromPath = sdPath + sdList.getItem(sdSel) + util::toUtf16("/");
                                    std::u16string toPath   = savePath + sdList.getItem(sdSel);
                                    if(confirmTransfer(fromPath, toPath, SD_TO_ARCH))
                                    {
                                        FSUSER_CreateDirectory(fs::getSaveArch(), fsMakePath(PATH_UTF16, toPath.data()), 0);
                                        toPath += util::toUtf16("/");

                                        fs::copyDirToArch(fs::getSaveArch(), fromPath, toPath);
                                    }
                                }
                                else
                                {
                                    std::u16string fromPath = sdPath + sdList.getItem(sdSel);
                                    std::u16string toPath   = savePath + sdList.getItem(sdSel);

                                    if(confirmTransfer(fromPath, toPath, SD_TO_ARCH))
                                        fs::copyFileToArch(fs::getSaveArch(), fromPath, toPath);
                                }
                                fs::commitData(fs::getSaveMode());
                            }
                        }
                        break;

                }
            }
            break;

        //Delete
        case 1:
            {
                switch(advPrev)
                {
                    case 0://Save was active
                        if(saveMenu.getSelected() == 0)
                        {
                            if(confirmDelete(savePath, ARCH_TO_SD))
                            {
                                FSUSER_DeleteDirectoryRecursively(fs::getSaveArch(), fsMakePath(PATH_UTF16, savePath.data()));
                                fs::commitData(fs::getSaveMode());
                            }
                        }
                        else if(saveMenu.getSelected() > 1)
                        {
                            int saveSel = saveMenu.getSelected() - 2;
                            if(saveList.isDir(saveSel))
                            {
                                std::u16string delPath = savePath + saveList.getItem(saveSel);
                                if(confirmDelete(delPath, ARCH_TO_SD))
                                    FSUSER_DeleteDirectoryRecursively(fs::getSaveArch(), fsMakePath(PATH_UTF16, delPath.data()));
                            }
                            else
                            {
                                std::u16string delPath = savePath + saveList.getItem(saveSel);
                                if(confirmDelete(delPath, ARCH_TO_SD))
                                    FSUSER_DeleteFile(fs::getSaveArch(), fsMakePath(PATH_UTF16, delPath.data()));
                            }
                            fs::commitData(fs::getSaveMode());
                        }
                        break;

                    case 1://SD was active
                        if(sdMenu.getSelected() == 0 && sdPath != util::toUtf16("/"))//I'm going to block it before it's my fault
                        {
                            if(confirmDelete(sdPath, SD_TO_ARCH))
                                FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), fsMakePath(PATH_UTF16, sdPath.data()));
                        }
                        else if(sdMenu.getSelected() > 1)
                        {
                            int sdSel = sdMenu.getSelected() - 2;
                            if(sdList.isDir(sdSel))
                            {
                                std::u16string delPath = sdPath + sdList.getItem(sdSel);
                                if(confirmDelete(delPath, SD_TO_ARCH))
                                    FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), fsMakePath(PATH_UTF16, delPath.data()));
                            }
                            else
                            {
                                std::u16string delPath = sdPath + sdList.getItem(sdSel);
                                if(confirmDelete(delPath, SD_TO_ARCH))
                                    FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_UTF16, delPath.data()));
                            }
                        }
                        break;
                }
            }
            break;

        //Rename
        case 2:
            {
                switch(advPrev)
                {
                    case 0:
                        {
                            std::u16string newName;
                            if(saveMenu.getSelected() > 1 && !(newName = util::toUtf16(util::getString("New Name", false))).empty())
                            {
                                int sel = saveMenu.getSelected() - 2;
                                std::u16string oldPath = savePath + saveList.getItem(sel);
                                std::u16string newPath = savePath + newName;

                                if(saveList.isDir(sel))
                                    FSUSER_RenameDirectory(fs::getSaveArch(), fsMakePath(PATH_UTF16, oldPath.data()), fs::getSaveArch(), fsMakePath(PATH_UTF16, newPath.data()));
                                else
                                    FSUSER_RenameFile(fs::getSaveArch(), fsMakePath(PATH_UTF16, oldPath.data()), fs::getSaveArch(), fsMakePath(PATH_UTF16, newPath.data()));

                                fs::commitData(fs::getSaveMode());
                            }
                        }
                        break;

                    case 1:
                        {
                            std::u16string newName;
                            if(sdMenu.getSelected() > 1 && !(newName = util::toUtf16(util::getString("New Name", false))).empty())
                            {
                                int sel = sdMenu.getSelected() - 2;
                                std::u16string oldPath = sdPath + sdList.getItem(sel);
                                std::u16string newPath = sdPath + newName;

                                if(sdList.isDir(sel))
                                    FSUSER_RenameDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, oldPath.data()), fs::getSDMCArch(), fsMakePath(PATH_UTF16, newPath.data()));
                                else
                                    FSUSER_RenameFile(fs::getSDMCArch(), fsMakePath(PATH_UTF16, oldPath.data()), fs::getSDMCArch(), fsMakePath(PATH_UTF16, newPath.data()));
                            }
                        }
                        break;
                }
            }
            break;

        //mkdir
        case 3:
            {
                std::u16string newDir;
                if(!(newDir = util::toUtf16(util::getString("New Dir", false))).empty())
                {
                    switch(advPrev)
                    {
                        case 0:
                            {
                                std::u16string crPath = savePath + newDir;
                                FSUSER_CreateDirectory(fs::getSaveArch(), fsMakePath(PATH_UTF16, crPath.data()), 0);
                                fs::commitData(fs::getSaveMode());
                            }
                            break;

                        case 1:
                            {
                                std::u16string crPath = sdPath + newDir;
                                FSUSER_CreateDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, crPath.data()), 0);
                            }
                            break;
                    }
                }
            }
            break;

        //Del SV
        case 4:
            fs::deleteSv(fs::getSaveMode());
            break;

        case 5:
            advMenuCtrl = advPrev;
            break;

    }

    //Update lists + menus
    saveList.reassign(fs::getSaveArch(), savePath);
    util::copyDirlistToMenu(saveList, saveMenu);
    sdList.reassign(fs::getSDMCArch(), sdPath);
    util::copyDirlistToMenu(sdList, sdMenu);
}

namespace ui
{
    void advModePrep()
    {
        advMenuCtrl = 0;

        savePath = util::toUtf16("/");
        sdPath   = util::toUtf16("/");

        saveList.reassign(fs::getSaveArch(), savePath);
        sdList.reassign(fs::getSDMCArch(), sdPath);

        util::copyDirlistToMenu(saveList, saveMenu);
        util::copyDirlistToMenu(sdList, sdMenu);

        copyMenu.reset();
        copyMenu.addOpt("Copy From", 0);
        copyMenu.addOpt("Delete", 0);
        copyMenu.addOpt("Rename", 0);
        copyMenu.addOpt("Make Dir", 0);
        copyMenu.addOpt("Delete SV", 0);
        copyMenu.addOpt("Back", 0);
    }

    void stateAdvMode(const uint64_t& down, const uint64_t& held)
    {
        switch(advMenuCtrl)
        {
            case 0://Save
                saveMenu.handleInput(down, held);
                break;

            case 1://SD
                sdMenu.handleInput(down, held);
                break;

            case 2://Cpy
                copyMenu.handleInput(down, held);
                break;
        }

        if(down & KEY_L || down & KEY_R)
        {
            if(advMenuCtrl == 0)
                advMenuCtrl = 1;
            else if(advMenuCtrl == 1)
                advMenuCtrl = 0;
        }
        else if(down & KEY_A)
        {
            switch(advMenuCtrl)
            {
                case 0://Save
                    {
                        int saveSel = saveMenu.getSelected();
                        if(saveSel == 1 && savePath != util::toUtf16("/"))
                        {
                            util::removeLastDirFromString(savePath);
                            savePathDisp = util::toUtf8(savePath);

                            saveList.reassign(fs::getSaveArch(), savePath);
                            util::copyDirlistToMenu(saveList, saveMenu);
                            saveMenu.setSelected(0);
                        }
                        else if(saveSel > 1 && saveList.isDir(saveSel - 2))
                        {
                            savePath += saveList.getItem(saveSel - 2) + util::toUtf16("/");
                            savePathDisp = util::toUtf8(savePath);

                            saveList.reassign(fs::getSaveArch(), savePath);
                            util::copyDirlistToMenu(saveList, saveMenu);
                            saveMenu.setSelected(0);
                        }
                    }
                    break;

                case 1://SD
                    {
                        int sdSel = sdMenu.getSelected();
                        if(sdSel == 1 && sdPath != util::toUtf16("/"))
                        {
                            util::removeLastDirFromString(sdPath);
                            sdPathDisp = util::toUtf8(sdPath);

                            sdList.reassign(fs::getSDMCArch(), sdPath);
                            util::copyDirlistToMenu(sdList, sdMenu);
                            sdMenu.setSelected(0);
                        }
                        else if(sdSel > 1 && sdList.isDir(sdSel - 2))
                        {
                            sdPath += sdList.getItem(sdSel - 2) + util::toUtf16("/");
                            sdPathDisp = util::toUtf8(sdPath);

                            sdList.reassign(fs::getSDMCArch(), sdPath);
                            util::copyDirlistToMenu(sdList, sdMenu);
                            sdMenu.setSelected(0);
                        }
                    }
                    break;

                case 2://Copy
                    performCopyOps();
                    break;
            }
        }
        else if(down & KEY_B)
        {
            if(advMenuCtrl == 0 && savePath != util::toUtf16("/"))
            {
                util::removeLastDirFromString(savePath);
                savePathDisp = util::toUtf8(savePath);

                saveList.reassign(fs::getSaveArch(), savePath);
                util::copyDirlistToMenu(saveList, saveMenu);
                saveMenu.setSelected(0);
            }
            else if(advMenuCtrl == 1 && sdPath != util::toUtf16("/"))
            {
                util::removeLastDirFromString(sdPath);
                sdPathDisp = util::toUtf8(sdPath);

                sdList.reassign(fs::getSDMCArch(), sdPath);
                util::copyDirlistToMenu(sdList, sdMenu);
                sdMenu.setSelected(0);
            }
            else if(advMenuCtrl == 2)
                advMenuCtrl = advPrev;
        }
        else if(down & KEY_X)
        {
            if(advMenuCtrl == 2)
                advMenuCtrl = advPrev;
            else
            {
                advPrev = advMenuCtrl;
                advMenuCtrl = 2;
            }
        }

        else if(down & KEY_SELECT)
            state = FLDR_MENU;

        gfx::frameBegin();
        gfx::frameStartTop();
        ui::drawTopBar("Adv. Mode");
        gfx::drawU16Text(util::toUtf16("sv:") + savePath, 0, 20, 0xFFFFFFFF);
        saveMenu.draw(40, 32, 0xFFFFFFFF, 320, false);
        if(advMenuCtrl == 2 && advPrev == 0)
        {
            copyMenu.editOpt(0, "Copy to SD");
            C2D_DrawRectSolid(144, 62, 0.5f, 112, 120, 0xFFEBEBEB);
            copyMenu.draw(152, 70, 0xFF000000, 96, true);
        }
        gfx::frameStartBot();
        gfx::drawU16Text(util::toUtf16("sd:") + sdPath, 0, 0, 0xFFFFFFFF);
        sdMenu.draw(0, 24, 0xFFFFFFFF, 320, false);
        if(advMenuCtrl == 2 && advPrev == 1)
        {
            copyMenu.editOpt(0, "Copy to Save");
            C2D_DrawRectSolid(100, 62, 0.5f, 112, 120, 0xFFEBEBEB);
            copyMenu.draw(108, 70, 0xFF000000, 96, true);
        }
        gfx::frameEnd();
    }
}

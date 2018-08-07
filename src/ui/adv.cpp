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

    std::string conf = "Are you sure you want to copy \"" + fromDrive + util::toUtf8(from) + "\" to \"" + toDrive + util::toUtf8(to) + "\"?";

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
                                    fs::copyDirToArch(fs::getSaveArch(), sdPath, savePath);
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
                            }
                        }
                        break;

                }
            }
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
                        }
                        else if(saveSel > 1 && saveList.isDir(saveSel - 2))
                        {
                            savePath += saveList.getItem(saveSel - 2) + util::toUtf16("/");
                            savePathDisp = util::toUtf8(savePath);

                            saveList.reassign(fs::getSaveArch(), savePath);
                            util::copyDirlistToMenu(saveList, saveMenu);
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
                        }
                        else if(sdSel > 1 && sdList.isDir(sdSel - 2))
                        {
                            sdPath += sdList.getItem(sdSel - 2) + util::toUtf16("/");
                            sdPathDisp = util::toUtf8(sdPath);

                            sdList.reassign(fs::getSDMCArch(), sdPath);
                            util::copyDirlistToMenu(sdList, sdMenu);
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
            }
            else if(advMenuCtrl == 1 && sdPath != util::toUtf16("/"))
            {
                util::removeLastDirFromString(sdPath);
                sdPathDisp = util::toUtf8(sdPath);

                sdList.reassign(fs::getSDMCArch(), sdPath);
                util::copyDirlistToMenu(sdList, sdMenu);
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
        saveMenu.draw(40, 32, 0xFFFFFFFF, 320);
        if(advMenuCtrl == 2 && advPrev == 0)
        {
            C2D_DrawRectSolid(144, 78, 0.5f, 112, 88, 0xFFCCCCCC);
            copyMenu.draw(152, 86, 0xFF000000, 96);
        }
        gfx::frameStartBot();
        sdMenu.draw(0, 24, 0xFFFFFFFF, 320);
        if(advMenuCtrl == 2 && advPrev == 1)
        {
            C2D_DrawRectSolid(100, 78, 0.5f, 112, 88, 0xFFCCCCCC);
            copyMenu.draw(108, 86, 0xFF000000, 96);
        }
        gfx::frameEnd();
    }
}

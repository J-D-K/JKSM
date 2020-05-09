#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>

#include <stdio.h>

#include "data.h"
#include "gfx.h"
#include "ui.h"
#include "util.h"
#include "fs.h"
#include "sys.h"

#include "smdh.h"

static ui::menu mainMenu, titleMenu, backupMenu, nandMenu, nandBackupMenu, folderMenu;

int state = MAIN_MENU, prev = MAIN_MENU;

namespace ui
{
    void loadTitleMenu()
    {
        titleMenu.reset();
        titleMenu.multiSet(true);
        for(unsigned i = 0; i < data::titles.size(); i++)
        {
            if(data::titles[i].getFav())
                titleMenu.addOpt("♥ " + util::toUtf8(data::titles[i].getTitle()), 320);
            else
                titleMenu.addOpt(util::toUtf8(data::titles[i].getTitle()), 320);
        }

        titleMenu.adjust();
    }

    void loadNandMenu()
    {
        nandMenu.reset();
        for(unsigned i = 0; i < data::nand.size(); i++)
            nandMenu.addOpt(util::toUtf8(data::nand[i].getTitle()), 320);
    }

    void prepMenus()
    {
        mainMenu.addOpt("Titles", 0);
        mainMenu.addOpt("System Titles", 0);
        mainMenu.addOpt("Reload Titles", 0);
        mainMenu.addOpt("Play Coins", 0);
        mainMenu.addOpt("Exit", 0);

        //Title menu
        loadTitleMenu();
        loadNandMenu();

        backupMenu.addOpt("Save Data", 0);
        backupMenu.addOpt("Delete Save Data", 0);
        backupMenu.addOpt("Extra Data", 0);
        backupMenu.addOpt("Delete Extra Data", 0);
        backupMenu.addOpt("Back", 0);

        nandBackupMenu.addOpt("System Save", 0);
        nandBackupMenu.addOpt("Extra Data", 0);
        nandBackupMenu.addOpt("BOSS Extra Data", 0);
        nandBackupMenu.addOpt("Back", 0);
    }

    void drawTopBar(const std::string& info)
    {
        C2D_DrawRectSolid(0, 0, 0.5f, 400, 16, 0xFF505050);
        C2D_DrawRectSolid(0, 17, 0.5f, 400, 1, 0xFF1D1D1D);
        gfx::drawText(info, 4, 0, 0xFFFFFFFF);
    }

    void stateMainMenu(const uint32_t& down, const uint32_t& held)
    {
        mainMenu.handleInput(down, held);

        if(down & KEY_A)
        {
            switch(mainMenu.getSelected())
            {
                case 0:
                    //do a cart check first
                    data::cartCheck();
                    if(!data::titles.empty())
                        state = TITLE_MENU;
                    else
                        ui::showMessage("No titles found. Try reloading titles!");
                    break;

                case 1:
                    state = SYS_MENU;
                    break;

                case 2:
                    remove("/JKSV/titles");
                    remove("/JKSV/nand");
                    data::loadTitles();
                    data::loadNand();
                    loadTitleMenu();
                    break;

                case 3:
                    util::setPC();
                    break;

                case 4:
                    sys::run = false;
                    break;
            }
        }
        else if(down & KEY_B)
            sys::run = false;

        gfx::frameBegin();
        gfx::frameStartTop();
        drawTopBar("JKSM - 05.08.2020");
        mainMenu.draw(40, 82, 0xFFFFFFFF, 320, false);
        gfx::frameStartBot();
        gfx::frameEnd();
    }

    void stateTitleMenu(const uint32_t& down, const uint32_t& held)
    {
        data::cartCheck();

        //Kick back if empty
        if(data::titles.empty())
        {
            state = MAIN_MENU;
            return;
        }

        //Much needed Jump button
        static ui::button jumpTo("Jump To", 0, 208, 320, 32);
        //Dump all button
        static ui::button dumpAll("Dump All", 0, 174, 320, 32);
        //Blacklist button
        static ui::button bl("Add to Blacklist \ue002", 0, 140, 320, 32);
        //Selected Dump
        static ui::button ds("Dump Selected \ue003", 0, 106, 320, 32);
        //Favorite
        static ui::button fav("Favorite \ue005", 0, 72, 320, 32);

        titleMenu.handleInput(down, held);

        touchPosition p;
        hidTouchRead(&p);

        jumpTo.update(p);
        dumpAll.update(p);
        bl.update(p);
        ds.update(p);
        fav.update(p);

        if(down & KEY_A)
        {
            data::curData = data::titles[titleMenu.getSelected()];

            state = BACK_MENU;
        }
        else if(down & KEY_B)
        {
            titleMenu.setSelected(0);
            state = MAIN_MENU;
        }
        else if(down & KEY_X || bl.getEvent() == BUTTON_RELEASED)
        {
            std::string confString = "Are you sure you want to add '" + util::toUtf8(data::titles[titleMenu.getSelected()].getTitle()) + "' to the blacklist?";
            if(confirm(confString))
                data::blacklistAdd(data::titles[titleMenu.getSelected()]);
        }
        else if(down & KEY_Y || ds.getEvent() == BUTTON_RELEASED)
        {
            for(unsigned i = 0; i < titleMenu.getCount(); i++)
            {
                if(titleMenu.multiIsSet(i) && fs::openArchive(data::titles[i], ARCHIVE_USER_SAVEDATA, false))
                {
                    util::createTitleDir(data::titles[i], ARCHIVE_USER_SAVEDATA);
                    std::u16string outpath = util::createPath(data::titles[i], ARCHIVE_USER_SAVEDATA) + util::toUtf16(util::getDateString(util::DATE_FMT_YMD));
                    FSUSER_CreateDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, outpath.data()), 0);
                    outpath += util::toUtf16("/");

                    fs::backupArchive(outpath);
                    fs::closeSaveArch();
                }

                if(titleMenu.multiIsSet(i) && fs::openArchive(data::titles[i], ARCHIVE_EXTDATA, false))
                {
                    util::createTitleDir(data::titles[i], ARCHIVE_EXTDATA);
                    std::u16string outpath = util::createPath(data::titles[i], ARCHIVE_EXTDATA) + util::toUtf16(util::getDateString(util::DATE_FMT_YMD));
                    FSUSER_CreateDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, outpath.data()), 0);
                    outpath += util::toUtf16("/");

                    fs::backupArchive(outpath);
                    fs::closeSaveArch();
                }
            }
        }
        else if(down & KEY_R || fav.getEvent() == BUTTON_RELEASED)
        {
            if(data::titles[titleMenu.getSelected()].getFav())
                data::favRem(data::titles[titleMenu.getSelected()]);
            else
                data::favAdd(data::titles[titleMenu.getSelected()]);
        }
        else if(jumpTo.getEvent() == BUTTON_RELEASED)
        {
            char16_t getChar = util::toUtf16(util::getString("Enter a letter to jump to", false))[0];
            if(getChar != 0x00)
            {
                unsigned i;
                if(data::titles[0].getMedia() == MEDIATYPE_GAME_CARD)
                    i = 1;
                else
                    i = 0;

                for( ; i < titleMenu.getCount(); i++)
                {
                    if(std::tolower(data::titles[i].getTitle()[0]) == getChar)
                    {
                        titleMenu.setSelected(i);
                        break;
                    }
                }
            }
        }
        else if(dumpAll.getEvent() == BUTTON_RELEASED)
        {
            fs::backupAll();
        }

        gfx::frameBegin();
        gfx::frameStartTop();
        drawTopBar("Select a Title");
        titleMenu.draw(40, 24, 0xFFFFFFFF, 320, false);
        gfx::frameStartBot();
        data::titles[titleMenu.getSelected()].drawInfo(8, 8);
        jumpTo.draw();
        dumpAll.draw();
        bl.draw();
        ds.draw();
        fav.draw();
        gfx::frameEnd();
    }

    void prepFolderMenu(data::titleData& dat, const uint32_t& mode)
    {
        std::u16string path = util::createPath(dat, mode);

        fs::dirList bakDir(fs::getSDMCArch(), path);

        folderMenu.reset();
        folderMenu.addOpt("New", 0);

        for(unsigned i = 0; i < bakDir.getCount(); i++)
            folderMenu.addOpt(util::toUtf8(bakDir.getItem(i)), 320);

        folderMenu.adjust();
    }

    void stateBackupMenu(const uint32_t& down, const uint32_t& held)
    {
        backupMenu.handleInput(down, held);

        if(down & KEY_A)
        {
            switch(backupMenu.getSelected())
            {
                case 0:
                    if(fs::openArchive(data::curData, ARCHIVE_USER_SAVEDATA, true))
                    {
                        util::createTitleDir(data::curData, ARCHIVE_USER_SAVEDATA);
                        prepFolderMenu(data::curData, ARCHIVE_USER_SAVEDATA);
                        prev  = BACK_MENU;
                        state = FLDR_MENU;
                    }
                    break;

                case 1:
                    if(confirm(std::string("Are you 100% sure you want to delete the current save data for this game? This is permanent!")) && fs::openArchive(data::curData, ARCHIVE_USER_SAVEDATA, true))
                    {
                        FSUSER_DeleteDirectoryRecursively(fs::getSaveArch(), fsMakePath(PATH_ASCII, "/"));
                        fs::commitData(ARCHIVE_USER_SAVEDATA);
                        fs::closeSaveArch();
                    }
                    break;

                case 2:
                    if(fs::openArchive(data::curData, ARCHIVE_EXTDATA, true))
                    {
                        util::createTitleDir(data::curData, ARCHIVE_EXTDATA);
                        prepFolderMenu(data::curData, ARCHIVE_EXTDATA);
                        prev  = BACK_MENU;
                        state = FLDR_MENU;
                    }
                    break;

                case 3:
                    {
                        std::string confStr = "Are you 100% sure you want to delete the currently saved Extra Data for '" + util::toUtf8(data::curData.getTitle()) + "'?";
                        if(confirm(confStr))
                        {
                            FS_ExtSaveDataInfo del = { MEDIATYPE_SD, 0, 0, data::curData.getExtData(), 0 };

                            Result res = FSUSER_DeleteExtSaveData(del);
                            if(R_SUCCEEDED(res))
                                showMessage("Extdata deleted!");
                        }
                    }
                    break;

                case 4:
                    backupMenu.setSelected(0);
                    state = TITLE_MENU;
                    break;
            }
        }
        else if(down & KEY_B)
        {
            backupMenu.setSelected(0);
            state = TITLE_MENU;
        }

        gfx::frameBegin();
        gfx::frameStartTop();
        drawTopBar(util::toUtf8(data::curData.getTitle()));
        backupMenu.draw(40, 82, 0xFFFFFFFF, 320, false);
        gfx::frameStartBot();
        gfx::frameEnd();
    }

    void stateNandMenu(const uint32_t& down, const uint32_t& held)
    {
        nandMenu.handleInput(down, held);

        if(down & KEY_A)
        {
            data::curData = data::nand[nandMenu.getSelected()];
            state = SYS_BAKMENU;
        }
        else if(down & KEY_B)
        {
            nandMenu.setSelected(0);
            state = MAIN_MENU;
        }

        gfx::frameBegin();
        gfx::frameStartTop();
        drawTopBar("Select a NAND Title");
        nandMenu.draw(40, 24, 0xFFFFFFFF, 320, false);
        gfx::frameStartBot();
        data::nand[nandMenu.getSelected()].drawInfo(8, 8);
        gfx::frameEnd();
    }

    void stateNandBack(const uint32_t& down, const uint32_t& held)
    {
        nandBackupMenu.handleInput(down, held);

        if(down & KEY_A)
        {
            switch(nandBackupMenu.getSelected())
            {
                case 0:
                    if(fs::openArchive(data::curData, ARCHIVE_SYSTEM_SAVEDATA, true))
                    {
                        util::createTitleDir(data::curData, ARCHIVE_SYSTEM_SAVEDATA);
                        prepFolderMenu(data::curData, ARCHIVE_SYSTEM_SAVEDATA);
                        prev  = SYS_BAKMENU;
                        state = FLDR_MENU;
                    }
                    break;

                case 1:
                    if(fs::openArchive(data::curData, ARCHIVE_EXTDATA, true))
                    {
                        util::createTitleDir(data::curData, ARCHIVE_EXTDATA);
                        prepFolderMenu(data::curData, ARCHIVE_EXTDATA);
                        prev  = SYS_BAKMENU;
                        state = FLDR_MENU;
                    }
                    break;

                case 2:
                    if(fs::openArchive(data::curData, ARCHIVE_BOSS_EXTDATA, true))
                    {
                        util::createTitleDir(data::curData, ARCHIVE_BOSS_EXTDATA);
                        prepFolderMenu(data::curData, ARCHIVE_BOSS_EXTDATA);
                        prev  = SYS_BAKMENU;
                        state = FLDR_MENU;
                    }
                    break;

                case 3:
                    nandBackupMenu.setSelected(0);
                    state = SYS_MENU;
                    break;
            }
        }
        else if(down & KEY_B)
        {
            nandBackupMenu.setSelected(0);
            state = SYS_MENU;
        }

        gfx::frameBegin();
        gfx::frameStartTop();
        drawTopBar(util::toUtf8(data::curData.getTitle()));
        nandBackupMenu.draw(40, 88, 0xFFFFFFFF, 320, false);
        gfx::frameStartBot();
        gfx::frameEnd();
    }

    void stateFolderMenu(const uint32_t& down, const uint32_t& held)
    {
        folderMenu.handleInput(down, held);

        int sel = folderMenu.getSelected();

        if(down & KEY_A)
        {
            //New
            if(sel == 0)
            {
                std::u16string newFolder;
                if(held & KEY_L)
                    newFolder = util::toUtf16(util::getDateString(util::DATE_FMT_YDM));
                else if(held & KEY_R)
                    newFolder = util::toUtf16(util::getDateString(util::DATE_FMT_YMD));
                else
                    newFolder = util::safeString(util::toUtf16(util::getString("Enter a new folder name", true)));

                if(!newFolder.empty())
                {
                    std::u16string fullPath = util::createPath(data::curData, fs::getSaveMode()) + newFolder;
                    FSUSER_CreateDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, fullPath.data()), 0);
                    fullPath += util::toUtf16("/");

                    fs::backupArchive(fullPath);

                    prepFolderMenu(data::curData, fs::getSaveMode());
                }
            }
            else
            {
                sel--;

                fs::dirList titleDir(fs::getSDMCArch(), util::createPath(data::curData, fs::getSaveMode()));
                std::string confStr = "Are you sure you want to overwrite '" + util::toUtf8(titleDir.getItem(sel)) + "'?";
                if(ui::confirm(confStr))
                {
                    std::u16string fullPath = util::createPath(data::curData, fs::getSaveMode()) + titleDir.getItem(sel);

                    //Del
                    FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), fsMakePath(PATH_UTF16, fullPath.data()));

                    //Recreate
                    FSUSER_CreateDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, fullPath.data()), 0);

                    fullPath += util::toUtf16("/");
                    fs::backupArchive(fullPath);
                }
            }
        }
        else if(down &  KEY_Y && sel != 0)
        {
            sel--;
            fs::dirList titleDir(fs::getSDMCArch(), util::createPath(data::curData, fs::getSaveMode()));
            std::string confStr = "Are you sure you want to restore '" + util::toUtf8(titleDir.getItem(sel)) + "'?";
            if(confirm(confStr))
            {
                std::u16string restPath = util::createPath(data::curData, fs::getSaveMode()) + titleDir.getItem(sel) + util::toUtf16("/");

                //Wipe root
                FSUSER_DeleteDirectoryRecursively(fs::getSaveArch(), fsMakePath(PATH_ASCII, "/"));

                //Restore from restPath
                fs::restoreToArchive(restPath);
            }
        }
        else if(down & KEY_X && sel != 0)
        {
            sel--;
            fs::dirList titleDir(fs::getSDMCArch(), util::createPath(data::curData, fs::getSaveMode()));
            std::string confStr = "Are you sure you want to delete '" + util::toUtf8(titleDir.getItem(sel)) + "'?";
            if(confirm(confStr))
            {
                std::u16string delPath = util::createPath(data::curData, fs::getSaveMode()) + titleDir.getItem(sel);

                FSUSER_DeleteDirectoryRecursively(fs::getSDMCArch(), fsMakePath(PATH_UTF16, delPath.data()));

                prepFolderMenu(data::curData, fs::getSaveMode());
            }
        }
        else if(down & KEY_SELECT)
        {
            advModePrep();
            state = ADV_MENU;
        }
        else if(down & KEY_B)
        {
            folderMenu.setSelected(0);
            fs::closeSaveArch();
            state = prev;
        }

        gfx::frameBegin();
        gfx::frameStartTop();
        drawTopBar("Select a Folder");
        folderMenu.draw(40, 24, 0xFFFFFFFF, 320, false);
        gfx::frameStartBot();
        gfx::drawText("\ue000 = Select\n\ue003 = Restore\n\ue002 = Delete\nSel. = File Mode\n\ue001 = Back", 16, 16, 0xFFFFFFFF);
        gfx::frameEnd();
    }

    void runApp(const uint32_t& down, const uint32_t& held)
    {
        switch(state)
        {
            case MAIN_MENU:
                stateMainMenu(down, held);
                break;

            case TITLE_MENU:
                stateTitleMenu(down, held);
                break;

            case BACK_MENU:
                stateBackupMenu(down, held);
                break;

            case SYS_MENU:
                stateNandMenu(down, held);
                break;

            case SYS_BAKMENU:
                stateNandBack(down, held);
                break;

            case FLDR_MENU:
                stateFolderMenu(down, held);
                break;

            case ADV_MENU:
                stateAdvMode(down, held);
                break;
        }
    }

    void showMessage(const char *fmt, ...)
    {
        char tmp[512];
        va_list args;
        va_start(args, fmt);
        vsprintf(tmp, fmt, args);

        ui:: button ok("OK \ue000", 96, 192, 128, 32);
        while(1)
        {
            hidScanInput();

            uint32_t down = hidKeysDown();
            touchPosition p;
            hidTouchRead(&p);

            ok.update(p);

            if(down & KEY_A || ok.getEvent() == BUTTON_RELEASED)
                break;

            gfx::frameBegin();
            gfx::frameStartBot();
            C2D_DrawRectSolid(8, 8, 0.5f, 304, 224, 0xFFE7E7E7);
            ok.draw();
            gfx::drawTextWrap(tmp, 16, 16, 224, 0xFF000000);
            gfx::frameEnd();
        }
    }

    progressBar::progressBar(const uint32_t& _max)
    {
        max = (float)_max;
    }

    void progressBar::update(const uint32_t& _prog)
    {
        prog = (float)_prog;

        float percent = (float)(prog / max) * 100;
        width  = (float)(percent * 288) / 100;
    }

    void progressBar::draw(const std::string& text)
    {
        C2D_DrawRectSolid(8, 8, 0.5f, 304, 224, 0xFFE7E7E7);
        C2D_DrawRectSolid(16, 200, 0.5f, 288, 16, 0xFF000000);
        C2D_DrawRectSolid(16, 200, 0.5f, width, 16, 0xFF00FF00);
        gfx::drawTextWrap(text, 16, 16, 224, 0xFF000000);
    }

    bool confirm(const std::string& mess)
    {
        button yes("Yes \ue000", 16, 192, 128, 32);
        button no("No \ue001", 176, 192, 128, 32);

        while(true)
        {
            hidScanInput();

            uint32_t down = hidKeysDown();
            touchPosition p;
            hidTouchRead(&p);

            //Oops
            yes.update(p);
            no.update(p);

            if(down & KEY_A || yes.getEvent() == BUTTON_RELEASED)
                return true;
            else if(down & KEY_B || no.getEvent() == BUTTON_RELEASED)
                return false;

            gfx::frameBegin();
            gfx::frameStartBot();
            C2D_DrawRectSolid(8, 8, 0.5f, 304, 224, 0xFFF4F4F4);
            gfx::drawTextWrap(mess, 16, 16, 224, 0xFF000000);
            yes.draw();
            no.draw();
            gfx::frameEnd();
        }
        return false;
    }
}

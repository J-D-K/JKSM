#include <3ds.h>
#include <vector>

#include "ui.h"
#include "data.h"
#include "util.h"
#include "gfx.h"

static ui::titleview *shrdView;
static bool fldOpen = false;
//These are hardcoded/specific
//Never change, don't need caching
static std::vector<data::titleData> shared;

//Stolen 3Dbrew descriptions
static const std::string sharedDesc[] =
{
    "NAND JPEG/MPO files and phtcache.bin from the camera application are stored here. This also contains UploadData.dat.",
    "NAND M4A files from the sound application are stored here.",
    "Used for SpotPass content storage for notifications.",
    "Contains idb.dat, idbt.dat, gamecoin.dat, ubll.lst, CFL_DB.dat, and CFL_OldDB.dat. These files contain cleartext Miis and some data relating (including cached ICN data) to Play/Usage Records.",
    "Contains bashotorya.dat and bashotorya2.dat.",
    "Home Menu SpotPass content data storage.",
    "Contains versionlist.dat, used by Home Menu for the software update notification added with 7.0.0-13."
};

static void fldCallback(void *)
{
    switch(ui::padKeysDown())
    {
        case KEY_B:
            fs::closeSaveArch();
            fldOpen = false;
            break;
    }
}

static void shrdViewCallback(void *)
{
    switch(ui::padKeysDown())
    {
        case KEY_A:
        {
            data::titleData *t = &shared[shrdView->getSelected()];
            if(fs::openArchive(*t, ARCHIVE_SHARED_EXTDATA, false))
            {
                util::createTitleDir(*t, ARCHIVE_SHARED_EXTDATA);
                std::u16string targetDir = util::createPath(*t, ARCHIVE_SHARED_EXTDATA);
                ui::fldInit(targetDir, fldCallback, NULL);
                fldOpen = true;
            }
        }
        break;

        case KEY_CPAD_LEFT:
            ui::state = BOS;
            break;

        case KEY_CPAD_RIGHT:
            ui::state = SET;
            break;
    }
}

void ui::shrdInit()
{
    //Skip E0 since it doesn't open
    data::titleData shrdExt;
    shrdExt.setExtdata(0xF0000001);
    shrdExt.setTitle(util::toUtf16("F0000001"));
    C2D_Image icon = util::createIconGeneric("F1", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF0000002);
    shrdExt.setTitle(util::toUtf16("F0000002"));
    icon = util::createIconGeneric("F2", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF0000009);
    shrdExt.setTitle(util::toUtf16("F0000009"));
    icon = util::createIconGeneric("F9", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000B);
    shrdExt.setTitle(util::toUtf16("F000000B"));
    icon = util::createIconGeneric("FB", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000C);
    shrdExt.setTitle(util::toUtf16("F000000C"));
    icon = util::createIconGeneric("FC", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000D);
    shrdExt.setTitle(util::toUtf16("F000000D"));
    icon = util::createIconGeneric("FD", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdExt.setExtdata(0xF000000E);
    shrdExt.setTitle(util::toUtf16("F000000E"));
    icon = util::createIconGeneric("FE", &gfx::iconSubTex);
    shrdExt.setIcon(icon);
    shared.push_back(shrdExt);

    shrdView = new titleview(shared, shrdViewCallback, NULL);
}

void ui::shrdExit()
{
    for(data::titleData& d : shared)
        d.freeIcon();

    delete shrdView;
}

void ui::shrdUpdate()
{
    if(fldOpen)
        ui::fldUpdate();
    else
        shrdView->update();
}

void ui::shrdDrawTop()
{
    ui::drawUIBar(TITLE_TEXT + "- Shared ExtData", ui::SCREEN_TOP, true);
    shrdView->draw();
}

void ui::shrdDrawBot()
{
    if(fldOpen)
        ui::fldDraw();

    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

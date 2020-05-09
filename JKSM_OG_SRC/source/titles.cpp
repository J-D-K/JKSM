#include <3ds.h>
#include <stdio.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>
#include <vector>
#include <ctype.h>
#include <algorithm>

#include "hbfilter.h"
#include "titles.h"
#include "db.h"
#include "ui.h"
#include "global.h"
#include "util.h"

std::vector<titleData> sdTitle;
std::vector<titleData> nandTitle;

//Not real alphabetical sorting, but close enough.
struct
{
    bool operator()(const titleData a, const titleData b)
    {
        for(unsigned i = 0; i < a.name.length(); i++)
        {
            int aChar = tolower(a.name[i]), bChar = tolower(b.name[i]);
            if(aChar != bChar)
                return aChar < bChar;
        }

        return false;
    }
} sortTitles;

bool checkHigh(u64 id)
{
    u32 high = id >> 32;
    //Games + Demos
    return (high == 0x00040000 || high == 0x00040002);
}

extern void prepSDSelect();

void sdTitlesInit()
{
    //for refresh games
    sdTitle.clear();
    if(fexists("titles"))
    {
        FILE *read = dbOpen("titles");

        //Check to make sure cache is updated since I changed it.
        if(dbGetRev(read) == 1)
        {
            u32 count = dbGetCount(read);
            sdTitle.reserve(count);

            for(unsigned i = 0; i < count; i++)
            {
                titleData newData = dbGetData(read);
                newData.media = MEDIATYPE_SD;
                sdTitle.push_back(newData);
            }

            fclose(read);
        }
        else
        {
            fclose(read);
            remove("titles");
            sdTitlesInit();
        }
    }
    else
    {
        //get title count for sdmc
        u32 count;
        AM_GetTitleCount(MEDIATYPE_SD, &count);
        sdTitle.reserve(count);

        //get ids
        u64 *ids = new u64[count];
        AM_GetTitleList(NULL, MEDIATYPE_SD, count, ids);

        progressBar load((float)count, "Installed SD Titles...", "Loading");
        for(unsigned i = 0; i < count; i++)
        {
            if( (checkHigh(ids[i]) && !hbFilter(ids[i])) || devMode)
            {
                titleData newTitle;
                if(newTitle.init(ids[i], MEDIATYPE_SD))
                    sdTitle.push_back(newTitle);
            }

            sf2d_start_frame(GFX_TOP, GFX_LEFT);
            sf2d_end_frame();

            sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
            load.draw((float)i);
            sf2d_end_frame();

            sf2d_swapbuffers();
        }
        delete[] ids;

        std::sort(sdTitle.begin(), sdTitle.end(), sortTitles);

        FILE *db = dbCreate("titles");
        dbWriteCount(db, sdTitle.size(), 1);
        for(unsigned i = 0; i < sdTitle.size(); i++)
            dbWriteData(db, sdTitle[i]);
        fclose(db);
    }

    prepSDSelect();
}

bool nandFilter(u64 id)
{
    u32 low = (u32)id;
    //camera applet
    if(low == 0x00009002 || low == 0x00008402 || low == 0x00009902 || low == 0x0000AA02 || low == 0x0000B202)
        return true;

    return false;
}

void sysSaveRedirect(titleData *dat)
{
    //this is for browser and ar games
    if(dat->low > 0x20000000)
        dat->unique = (0x0000FFFF & dat->unique);
    if(dat->low == 0x2002CF00)
    {
        dat->unique = 0x0000008F;
        dat->extdata = 0x0000008F;
    }
}

extern void prepNandSelect();

void nandTitlesInit()
{
    nandTitle.clear();
    if(fexists("nand"))
    {
        FILE *read = dbOpen("nand");

        if(dbGetRev(read) == 1)
        {
            u32 count = dbGetCount(read);
            nandTitle.reserve(count);

            for(unsigned i = 0; i < count; i++)
            {
                titleData newNand = dbGetData(read);
                newNand.media = MEDIATYPE_NAND;
                sysSaveRedirect(&newNand);
                nandTitle.push_back(newNand);
            }
            fclose(read);
        }
        else
        {
            fclose(read);
            remove("nand");
            nandTitlesInit();
        }
    }
    else
    {
        u32 count;
        AM_GetTitleCount(MEDIATYPE_NAND, &count);

        u64 *ids = new u64[count];
        AM_GetTitleList(NULL, MEDIATYPE_NAND, count, ids);

        progressBar load((float)count, "NAND Titles...", "Loading");
        for(unsigned i = 0; i < count; i++)
        {
            if(!(nandFilter(ids[i]) && ids[i] != 0) || devMode)
            {
                titleData newData;
                if( (newData.init(ids[i], MEDIATYPE_NAND) && !newData.name.empty()))
                {
                    sysSaveRedirect(&newData);
                    nandTitle.push_back(newData);
                }
            }

            sf2d_start_frame(GFX_TOP, GFX_LEFT);
            sf2d_end_frame();

            sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
            load.draw((float)i);
            sf2d_end_frame();

            sf2d_swapbuffers();
        }
        delete[] ids;

        std::sort(nandTitle.begin(), nandTitle.end(), sortTitles);

        FILE *nand = dbCreate("nand");
        dbWriteCount(nand, nandTitle.size(), 1);
        for(unsigned i = 0; i < nandTitle.size(); i++)
            dbWriteData(nand, nandTitle[i]);
        fclose(nand);
    }

    prepNandSelect();
}

#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <string>

#include "titledata.h"
#include "global.h"
#include "smdh.h"
#include "util.h"

u32 extdataRedirect(u32 low)
{
    //Pokemon Y
    if(low == 0x00055E00)
        return 0x0000055D;
    //Pokemon OR
    else if(low == 0x0011C400)
        return 0x000011C5;
    //Pokemon Moon
    else if(low == 0x00175E00)
        return 0x00001648;
    //Fire Emblem Conquest + SE NA
    else if(low == 0x00179600 || low == 0x00179800)
        return 0x00001794;
    //FE Conquest + SE EURO
    else if(low == 0x00179700 || low == 0x0017A800)
        return 0x00001795;
    //FE If/JPN
    else if(low == 0x0012DD00 || low == 0x0012DE00)
        return 0x000012DC;

    return (low >> 8);
}

bool titleData::init(u64 _id, FS_MediaType mediaType)
{
    id = _id;
    media = mediaType;

    //split id and setup other ids
    low = (u32)id;
    high = (u32)(id >> 32);
    unique = (low >> 8);

    //some titles use a different id for extdata
    extdata = extdataRedirect(low);

    smdh_s *smdh = loadSMDH(low, high, media);
    if(smdh == NULL)
        return false;

    name = (char16_t *)smdh->applicationTitles[sysLanguage].shortDescription;
    //Default to english if title is empty for language
    if(name.empty())
        name = (char16_t *)smdh->applicationTitles[1].shortDescription;
    u32Name = tou32(name);
    nameSafe = safeString(name);

    delete smdh;

    //Product code
    char tmp[16];
    AM_GetTitleProductCode(media, id, tmp);
    prodCode.assign(tmp);

    initd = true;

    return true;
}

void titleData::initId()
{
    high = id >> 32;
    low = id;

    unique = (low >> 8);

    extdata = extdataRedirect(low);

    media = MEDIATYPE_SD;

    nameSafe = safeString(name);
    u32Name = tou32(name);

    initd = true;
}

void titleData::printInfo()
{
    sftd_draw_textf(font, 0, 0, RGBA8(0, 255, 255, 255), 12, "High ID : %08X", high);
    sftd_draw_textf(font, 0, 14, RGBA8(255, 255, 0, 255), 12, "Low ID : %08X", low);
    sftd_draw_textf(font, 0, 28, RGBA8(0, 255, 0, 255), 12, "Prod. Code : %s", prodCode.c_str());
    if(media == MEDIATYPE_GAME_CARD)
        sftd_draw_text(font, 0, 42, RGBA8(255, 0, 0, 255), 12, "Game Card");
    else if(media == MEDIATYPE_SD)
        sftd_draw_text(font, 0, 42, RGBA8(255, 0, 0, 255), 12, "SD/CIA Title");
}

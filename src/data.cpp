#include <3ds.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <zlib.h>

#include "util.h"
#include "data.h"
#include "fs.h"
#include "ui.h"
#include "gfx.h"

#define ICON_BUFF_SIZE 0x2000

//Needed for icon sub tex. Top UV needs to be higher than bottom so it's rotated.
const Tex3DS_SubTexture subTex = {48, 48, 0.0f, 0.75f, 0.75f, 0.0f};

const char *blPath    = "/JKSV/blacklist.txt";
const char *favPath   = "/JKSV/favorites.txt";
const char *titlePath = "/JKSV/titles";
const char *nandPath  = "/JKSV/nand";

static uint32_t extdataRedirect(const uint32_t& low)
{
    switch(low)
    {
        //Pokemon Y
        case 0x00055E00:
            return 0x0000055D;
            break;

        //Pokemon OR
        case 0x0011C400:
            return 0x000011C5;
            break;

        //Pkmn moon
        case 0x00175E00:
            return 0x00001648;
            break;

        //Ultra moon
        case 0x001B5100:
            return 0x00001B50;
            break;

        //FE Conquest + SE NA
        case 0x00179600:
        case 0x00179800:
            return 0x00001794;
            break;

        //FE Conquest + SE Euro
        case 0x00179700:
        case 0x0017A800:
            return 0x00001795;
            break;

        //FE if JP
        case 0x0012DD00:
        case 0x0012DE00:
            return 0x000012DC;
            break;

        default:
            return low >> 8;
            break;
    }
}

std::vector<uint64_t> blacklist;
std::vector<uint64_t> favorites;

struct
{
    bool operator()(data::titleData& a, data::titleData& b)
    {
        if(a.getMedia() != b.getMedia())
            return a.getMedia() == MEDIATYPE_GAME_CARD;

        if(a.getFav() != b.getFav())
            return a.getFav() == true;

        for(unsigned i = 0; i < a.getTitle().length(); i++)
        {
            int aChar = tolower(a.getTitle()[i]), bChar = tolower(b.getTitle()[i]);
            if(aChar != bChar)
                return aChar < bChar;
        }

        return false;
    }
} sortTitles;

static bool isBlacklisted(const uint64_t& id)
{
    for(unsigned i = 0; i < blacklist.size(); i++)
    {
        if(id == blacklist[i])
            return true;
    }

    return false;
}
static bool isFavorite(const uint64_t& id)
{
    for(unsigned i = 0; i < favorites.size(); i++)
    {
        if(id == favorites[i])
            return true;
    }

    return false;
}

static C3D_Tex *loadIcon(smdh_s *smdh)
{
    C3D_Tex *ret = new C3D_Tex;
    uint16_t *icon = smdh->bigIconData;
    if(C3D_TexInit(ret, 64, 64, GPU_RGB565))
    {
        uint16_t *tex  = (uint16_t *)ret->data + (16 * 64);
        for(unsigned y = 0; y < 48; y += 8, icon += 48 *8, tex += 64 * 8)
            memcpy(tex, icon, sizeof(uint16_t) * 48 * 8);
    }
    return ret;
}

std::vector<data::titleData> data::titles;
std::vector<data::titleData> data::nand;
std::vector<uint32_t> filterIds;

uint8_t lang;

data::titleData data::curData;

void data::exit()
{
    for(auto t : data::titles)
        t.freeIcon();
}

bool data::titleData::init(const uint64_t& _id, const FS_MediaType& mt)
{
    m = mt;
    id = _id;

    low = (uint32_t)id;
    high = (uint32_t)(id >> 32);
    unique = (low >> 8);
    extdata = extdataRedirect(low);
    if(mt != MEDIATYPE_GAME_CARD && isFavorite(id))
        fav = true;

    smdh_s *smdh = loadSMDH(low, high, m);
    if(smdh == NULL)
        return false;

    title.assign((char16_t *)(smdh->applicationTitles[1].shortDescription));
    titleSafe.assign(util::safeString(title));
    icon = readIconFromSMDH(smdh);

    char tmp[16];
    AM_GetTitleProductCode(m, id, tmp);
    prodCode = tmp;

    delete smdh;

    return true;
}

bool data::titleData::initFromCache(const uint64_t& _id, const std::u16string& _title, const std::string& code, const uint8_t& mt)
{
    id = _id;
    low = (uint32_t)id;
    high = (uint32_t)(id >> 32);
    unique = (low >> 8);
    extdata = extdataRedirect(low);
    m = (FS_MediaType)mt;
    if(isFavorite(id))
        fav = true;

    title.assign(_title);
    titleSafe.assign(util::safeString(title));
    prodCode.assign(code);

    return true;
}

bool data::titleData::isOpenable()
{
    bool ret = false;

    if(getMedia() == MEDIATYPE_GAME_CARD || getMedia() == MEDIATYPE_SD)
    {
        ret = fs::openArchive(*this, ARCHIVE_USER_SAVEDATA, false);
        if(!ret)
            ret = fs::openArchive(*this, ARCHIVE_EXTDATA, false);
    }

    if(getMedia() == MEDIATYPE_NAND)
    {
        ret = fs::openArchive(*this, ARCHIVE_SYSTEM_SAVEDATA, false);

        if(!ret)
            ret = fs::openArchive(*this, ARCHIVE_EXTDATA, false);

        if(!ret)
            ret = fs::openArchive(*this, ARCHIVE_BOSS_EXTDATA, false);
    }

    fs::closeSaveArch();

    return ret;
}

void data::titleData::setTitle(const std::u16string& _t)
{
    title = _t;
    titleSafe = util::safeString(_t);
}

void data::titleData::drawInfo(unsigned x, unsigned y)
{
    std::string media;
    switch(getMedia())
    {
        case MEDIATYPE_GAME_CARD:
            media = "Game Card";
            break;

        case MEDIATYPE_SD:
            media = "SD";
            break;

        case MEDIATYPE_NAND:
            media = "NAND";
            break;
    }

    char tmp[64];
    sprintf(tmp, "Media: %s\nHigh: 0x%08X\nLow: 0x%08X", media.c_str(), (unsigned)getHigh(), (unsigned)getLow());
    gfx::drawText(tmp, x, y, 0xFFFFFFFF);
    if(icon.tex)
        C2D_DrawImageAt(icon, 160, 8, 0.5f);
}

void data::titleData::assignIcon(C3D_Tex *_icon)
{
    icon = {_icon, &subTex};
}

void data::cartCheck()
{
    bool ins = false;
    FSUSER_CardSlotIsInserted(&ins);

    if((titles.empty() || titles[0].getMedia() != MEDIATYPE_GAME_CARD) && ins)
    {
        uint64_t cartID = 0;
        AM_GetTitleList(NULL, MEDIATYPE_GAME_CARD, 1, &cartID);

        titleData cartData;
        if(cartData.init(cartID, MEDIATYPE_GAME_CARD))
        {
            titles.insert(titles.begin(), cartData);
            ui::loadTitleMenu();
        }
    }
    else if(titles[0].getMedia() == MEDIATYPE_GAME_CARD && !ins)
    {
        titles[0].freeIcon();
        titles.erase(titles.begin(), titles.begin() + 1);
        ui::loadTitleMenu();
    }
}

smdh_s *data::loadSMDH(const uint32_t& low, const uint32_t& high, const uint8_t& media)
{
    Handle handle;

    uint32_t archPath[] = {low, high, media, 0};
    static const uint32_t filePath[] = {0x0, 0x0, 0x2, 0x6E6F6369, 0x0};

    FS_Path binArchPath = {PATH_BINARY, 0x10, archPath};
    FS_Path binFilePath = {PATH_BINARY, 0x14, filePath};

    if(R_SUCCEEDED(FSUSER_OpenFileDirectly(&handle, ARCHIVE_SAVEDATA_AND_CONTENT, binArchPath, binFilePath, FS_OPEN_READ, 0)))
    {
        uint32_t read = 0;
        smdh_s *ret = new smdh_s;
        FSFILE_Read(handle, &read, 0, ret, sizeof(smdh_s));
        FSFILE_Close(handle);

        return ret;
    }

    return NULL;
}

static inline bool checkHigh(const uint64_t& id)
{
    uint32_t high = (uint32_t)(id >> 32);

    return (high == 0x00040000 || high == 0x00040002);
}

void data::loadTitles()
{
    titles.clear();
    loadBlacklist();
    loadFav();

    if(!readCache(titles, titlePath, false))
    {
        uint32_t count = 0;
        AM_GetTitleCount(MEDIATYPE_SD, &count);

        uint64_t *ids = new uint64_t[count];
        AM_GetTitleList(NULL, MEDIATYPE_SD, count, ids);

        ui::progressBar prog(count);

        for(unsigned i = 0; i < count; i++)
        {
            if(checkHigh(ids[i]) && !isBlacklisted(ids[i]))
            {
                titleData newTitle;
                if(newTitle.init(ids[i], MEDIATYPE_SD) && newTitle.isOpenable())
                    titles.push_back(newTitle);
            }

            prog.update(i);

            gfx::frameBegin();
            gfx::frameStartTop();
            ui::drawTopBar("Loading...");
            gfx::frameStartBot();
            prog.draw("Loading installed SD Titles...");
            gfx::frameEnd();
        }
        delete[] ids;

        std::sort(titles.begin(), titles.end(), sortTitles);

        createCache(titles, titlePath);
    }
    else
        std::sort(titles.begin(), titles.end(), sortTitles);
}

void data::loadNand()
{
    nand.clear();

    if(!readCache(nand,nandPath, true))
    {
        uint32_t count;
        AM_GetTitleCount(MEDIATYPE_NAND, &count);

        uint64_t *ids = new uint64_t[count];
        AM_GetTitleList(NULL, MEDIATYPE_NAND, count, ids);

        ui::progressBar prog(count);
        for(unsigned i = 0; i < count; i++)
        {
            titleData newNandTitle;
            if(newNandTitle.init(ids[i], MEDIATYPE_NAND) && newNandTitle.isOpenable() && !newNandTitle.getTitle().empty())
                nand.push_back(newNandTitle);

            prog.update(i);

            gfx::frameBegin();
            gfx::frameStartTop();
            ui::drawTopBar("Loading...");
            gfx::frameStartBot();
            prog.draw("Loading NAND Titles...");
            gfx::frameEnd();

        }
        delete[] ids;

        std::sort(nand.begin(), nand.end(), sortTitles);

        createCache(nand, nandPath);
    }
}

void data::loadBlacklist()
{
    blacklist.clear();
    if(util::fexists(blPath))
    {
        fs::fsfile bl(fs::getSDMCArch(), blPath, FS_OPEN_READ);

        char line[64];
        while(bl.getLine(line, 64))
        {
            if(line[0] == '#' || line[0] == '\n')
                continue;

            blacklist.push_back(strtoull(line, NULL, 16));
        }
    }
}

void data::saveBlacklist()
{
    fs::fsfile bl(fs::getSDMCArch(), blPath, FS_OPEN_CREATE | FS_OPEN_WRITE);
    for(unsigned i = 0; i < blacklist.size(); i++)
        bl.writef("0x%016llX\n", blacklist[i]);
}

void data::blacklistAdd(titleData& t)
{
    if(t.getMedia() == MEDIATYPE_GAME_CARD)
        return;

    blacklist.push_back(t.getID());

    //Remove it
    for(unsigned i = 0; i < titles.size(); i++)
    {
        if(titles[i].getID() == t.getID())
        {
            titles[i].freeIcon();
            titles.erase(titles.begin() + i);
            break;
        }
    }

    //Erase cart if it's there
    if(titles[0].getMedia() == MEDIATYPE_GAME_CARD)
        titles.erase(titles.begin());

    //Recreate cache with title missing now
    createCache(titles, titlePath);

    //Reinit title menu
    ui::loadTitleMenu();
}

void data::loadFav()
{
    if(util::fexists(favPath))
    {
        char line[64];
        fs::fsfile fav(fs::getSDMCArch(), favPath, FS_OPEN_READ);

        while(fav.getLine(line, 64))
        {
            if(line[0] == '#' || line[0] == '\n')
                continue;

            favorites.push_back(strtoull(line, NULL, 16));
        }
    }
}

void data::saveFav()
{
    if(favorites.size() > 0)
    {
        fs::fsfile fav(fs::getSDMCArch(), favPath, FS_OPEN_CREATE | FS_OPEN_WRITE);
        for(unsigned i = 0; i < favorites.size(); i++)
            fav.writef("0x%016llX\n", favorites[i]);
    }
}

void data::favAdd(titleData& t)
{
    t.setFav(true);

    favorites.push_back(t.getID());

    //resort with new fav
    std::sort(data::titles.begin(), data::titles.end(), sortTitles);
    //reload title menu
    ui::loadTitleMenu();
}

void data::favRem(titleData& t)
{
    t.setFav(false);

    unsigned i;
    for(i = 0; i < favorites.size(); i++)
    {
        if(favorites[i] == t.getID())
        {
            favorites.erase(favorites.begin() + i);
            break;
        }
    }

    std::sort(data::titles.begin(), data::titles.end(), sortTitles);
    ui::loadTitleMenu();
}

C2D_Image data::readIconFromSMDH(smdh_s *smdh)
{
    return (C2D_Image){loadIcon(smdh), &subTex};
}

void data::createCache(std::vector<titleData>& vect, const std::string& path)
{
    //JIC
    fs::fdelete(path);

    uint32_t writeOut = 0;
    fs::fsfile cache(fs::getSDMCArch(), path, FS_OPEN_CREATE | FS_OPEN_WRITE);

    //Buffer to compress icons
    size_t iconCmpSize = 0;
    uint8_t *iconOut = new uint8_t[ICON_BUFF_SIZE];

    uint16_t countOut = vect.size();
    cache.write(&countOut, &writeOut, sizeof(uint16_t));
    cache.putByte(0x03);

    for(auto t : vect)
    {
        char16_t titleOut[0x40];
        memset(titleOut, 0, 0x40 * 2);
        memcpy(titleOut, t.getTitle().data(), t.getTitle().length() * 2);
        cache.write(titleOut, &writeOut, 0x40 * 2);
        cache.putByte(0x00);

        char prodOut[16];
        memset(prodOut, 0, 16);
        memcpy(prodOut, t.getProdCode().data(), 16);
        cache.write(prodOut, &writeOut, 16);
        cache.putByte(0x00);

        uint64_t idOut = t.getID();
        cache.write(&idOut, &writeOut, sizeof(uint64_t));
        cache.putByte(0x00);

        iconCmpSize = ICON_BUFF_SIZE;
        compress(iconOut, (uLongf *)&iconCmpSize, t.getIconData(), ICON_BUFF_SIZE);
        cache.write(&iconCmpSize, &writeOut, sizeof(size_t));
        cache.write(iconOut, &writeOut, iconCmpSize);
        cache.putByte(0x00);
    }
    delete[] iconOut;
}

bool data::readCache(std::vector<titleData>& vect, const std::string& path, bool nand)
{
    if(!util::fexists(path))
        return false;

    fs::fsfile cache(fs::getSDMCArch(), path, FS_OPEN_READ);
    //Check revision
    uint8_t rev = 0;
    cache.seek(2, fs::seek_beg);
    rev = cache.getByte();
    cache.seek(0, fs::seek_beg);

    if(rev != 3)
        return false;

    uint32_t readOut = 0;
    uint16_t count = 0;
    cache.read(&count,&readOut, sizeof(uint16_t));
    cache.getByte();

    uint8_t *readBuff = new uint8_t[ICON_BUFF_SIZE];

    for(unsigned i = 0; i < count; i++)
    {
        titleData newData;

        char16_t title[0x40];
        cache.read(title, &readOut, 0x40 * sizeof(char16_t));
        cache.getByte();

        char prodCode[16];
        cache.read(prodCode, &readOut, 16);
        cache.getByte();

        uint64_t newID = 0;
        cache.read(&newID, &readOut, sizeof(uint64_t));
        cache.getByte();

        size_t iconSize = 0;
        cache.read(&iconSize, &readOut, sizeof(size_t));
        cache.read(readBuff, &readOut, iconSize);
        cache.getByte();

        C3D_Tex *icon = new C3D_Tex;
        if(C3D_TexInit(icon, 64, 64, GPU_RGB565))
        {
            uLongf sz = ICON_BUFF_SIZE;
            uncompress((uint8_t *)icon->data, &sz, readBuff, iconSize);
            newData.assignIcon(icon);
        }
        newData.initFromCache(newID, title, prodCode, nand ? MEDIATYPE_NAND : MEDIATYPE_SD);
        vect.push_back(newData);
    }

    delete[] readBuff;

    return true;
}

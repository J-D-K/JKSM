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
#include "type.h"

#define ICON_BUFF_SIZE 0x2000

const char *blPath    = "/JKSV/blacklist.txt";
const char *favPath   = "/JKSV/favorites.txt";
const char *titlePath = "/JKSV/cache.bin";
static uint8_t lang = CFG_LANGUAGE_EN;

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

std::vector<data::titleData> data::usrSaveTitles;
std::vector<data::titleData> data::extDataTitles;
std::vector<data::titleData> data::sysDataTitles;
std::vector<data::titleData> data::bossDataTitles;

//This is a master list now
static std::vector<data::titleData> titles;
std::vector<uint32_t> filterIds;
data::titleData data::curData;

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
    C3D_TexSetFilter(ret, GPU_LINEAR, GPU_LINEAR);
    uint16_t *icon = smdh->bigIconData;
    if(C3D_TexInit(ret, 64, 64, GPU_RGB565))//GPU can't use below 64x64
    {
        uint16_t *tex  = (uint16_t *)ret->data + (16 * 64);
        for(unsigned y = 0; y < 48; y += 8, icon += 48 *8, tex += 64 * 8)
            memcpy(tex, icon, sizeof(uint16_t) * 48 * 8);
    }
    return ret;
}

void data::init()
{
    CFGU_GetSystemLanguage(&lang);
}

void data::exit()
{
    for(auto t : titles)
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

    char tid[32];
    sprintf(tid, "%016llX", _id);
    idStr.assign(tid);

    char tmp[16];
    AM_GetTitleProductCode(m, id, tmp);
    prodCode = tmp;

    if(mt != MEDIATYPE_GAME_CARD && isFavorite(id))
        fav = true;

    testMounts();

    smdh_s *smdh = loadSMDH(low, high, m);
    if(smdh != NULL && hasSaveData())
    {
        title.assign((char16_t *)(smdh->applicationTitles[lang].shortDescription));
        if(title.empty())
            title.assign((char16_t *)(smdh->applicationTitles[1].shortDescription));

        titleUTF8 = util::toUtf8(title);

        titleSafe.assign(util::safeString(title));

        publisher.assign((char16_t *)(smdh->applicationTitles[lang].publisher));
        if(publisher.empty())
            publisher.assign((char16_t *)(smdh->applicationTitles[1].publisher));

        icon = readIconFromSMDH(smdh);
        delete smdh;
    }
    else if(hasSaveData())
    {
        title.assign(util::toUtf16(idStr));
        titleUTF8 = idStr;
        titleSafe.assign(util::toUtf16(idStr));
        publisher.assign(util::toUtf16("Someone?"));
        unsigned lowerFour = low & 0x0000FFFF;
        char tmp[16];
        sprintf(tmp, "%04X", lowerFour);
        icon = util::createIconGeneric(tmp, &gfx::iconSubTex);
    }

    return true;
}

bool data::titleData::initFromCache(const uint64_t& _id, const std::u16string& _title, const std::u16string& _pub, const std::string& code, const data::titleSaveTypes& _st, const uint8_t& mt)
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
    titleUTF8 = util::toUtf8(title);
    titleSafe.assign(util::safeString(title));
    publisher = _pub;
    if(publisher.empty())
        publisher = util::toUtf16("Someone?");
    prodCode.assign(code);
    types = _st;

    char tid[32];
    sprintf(tid, "%016llX", _id);
    idStr.assign(tid);

    return true;
}

void data::titleData::testMounts()
{
    if(getMedia() == MEDIATYPE_GAME_CARD || getMedia() == MEDIATYPE_SD)
    {
        if(fs::openArchive(*this, ARCHIVE_USER_SAVEDATA, false))
        {
               types.hasUser = true;
               fs::closeSaveArch();
        }

        if(fs::openArchive(*this, ARCHIVE_EXTDATA, false))
        {
            types.hasExt = true;
            fs::closeSaveArch();

        }
    }

    if(getMedia() == MEDIATYPE_NAND)
    {
        if(fs::openArchive(*this, ARCHIVE_SYSTEM_SAVEDATA, false))
        {
            types.hasSys = true;
            fs::closeSaveArch();
        }

        if(fs::openArchive(*this, ARCHIVE_EXTDATA, false))
        {
            types.hasExt = true;
            fs::closeSaveArch();
        }

        if(fs::openArchive(*this, ARCHIVE_BOSS_EXTDATA, false))
        {
            types.hasBoss = true;
            fs::closeSaveArch();
        }
    }
}

bool data::titleData::hasSaveData()
{
    return types.hasUser || types.hasExt || types.hasSys || types.hasBoss;
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

    C2D_DrawRectSolid(0, 0, GFX_DEPTH_DEFAULT, 320.0f, 16.0f, 0xFF505050);
    C2D_DrawRectSolid(0, 16, GFX_DEPTH_DEFAULT, 320.0f, 16.0f, 0xFF202020);
    C2D_DrawRectSolid(0, 32, GFX_DEPTH_DEFAULT, 320.0f, 16.0f, 0xFF505050);
    C2D_DrawRectSolid(0, 48, GFX_DEPTH_DEFAULT, 320.0f, 16.0f, 0xFF202020);
    C2D_DrawRectSolid(0, 64, GFX_DEPTH_DEFAULT, 320.0f, 16.0f, 0xFF505050);

    gfx::drawU16Text(title, 8, 0, GFX_DEPTH_DEFAULT, 0xFFFFFFFF);
    gfx::drawU16Text(publisher, 8, 16, GFX_DEPTH_DEFAULT, 0xFFFFFFFF);
    gfx::drawText(idStr, 8, 32, GFX_DEPTH_DEFAULT, 0.5f, 0xFFFFFFFF);
    gfx::drawText(prodCode, 8, 48, GFX_DEPTH_DEFAULT, 0.5f, 0xFFFFFFFF);
    gfx::drawText(media, 8,64, GFX_DEPTH_DEFAULT, 0.5f, 0xFFFFFFFF);
}

void data::titleData::drawIconAt(float x, float y, uint16_t w, uint16_t h, float depth)
{
    C2D_DrawImageAt(icon, x, y, depth);
}

void data::titleData::assignIcon(C3D_Tex *_icon)
{
    icon = {_icon, &gfx::iconSubTex};
}

static void loadcart(void *a)
{
    threadInfo *t = (threadInfo *)a;
    t->status->setStatus("Loading Cart Data...");
    uint64_t cartID = 0;
    AM_GetTitleList(NULL, MEDIATYPE_GAME_CARD, 1, &cartID);
    data::titleData cartData;
    if(cartData.init(cartID, MEDIATYPE_GAME_CARD))
    {
        data::titleSaveTypes tmp = cartData.getSaveTypes();
        if(tmp.hasUser)
            data::usrSaveTitles.insert(data::usrSaveTitles.begin(), cartData);

        if(tmp.hasExt)
            data::extDataTitles.insert(data::extDataTitles.begin(), cartData);

        ui::ttlRefresh();
        ui::extRefresh();
    }
    t->finished = true;
}

static bool checkForCart()
{
    return data::usrSaveTitles[0].getMedia() == MEDIATYPE_GAME_CARD || data::extDataTitles[0].getMedia() == MEDIATYPE_GAME_CARD;
}

void data::cartCheck()
{
    bool ins = false;
    FSUSER_CardSlotIsInserted(&ins);

    if(ins && !checkForCart())
        ui::newThread(loadcart, NULL, NULL);
    else if(!ins)
    {
        if(data::usrSaveTitles[0].getMedia() == MEDIATYPE_GAME_CARD)
        {
            data::usrSaveTitles[0].freeIcon();
            data::usrSaveTitles.erase(data::usrSaveTitles.begin());
            ui::ttlRefresh();
        }

        if(data::extDataTitles[0].getMedia() == MEDIATYPE_GAME_CARD)
        {
            data::extDataTitles[0].freeIcon();
            data::extDataTitles.erase(data::extDataTitles.begin());
            ui::extRefresh();
        }
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

void data::loadTitles(void *a)
{
    threadInfo *t = (threadInfo *)a;
    t->status->setStatus("Loading Titles...");

    titles.clear();
    usrSaveTitles.clear();
    extDataTitles.clear();
    sysDataTitles.clear();
    bossDataTitles.clear();
    loadBlacklist();
    loadFav();

    if(!readCache(titles, titlePath, false))
    {
        uint32_t count = 0;
        AM_GetTitleCount(MEDIATYPE_SD, &count);

        uint64_t *ids = new uint64_t[count];
        AM_GetTitleList(NULL, MEDIATYPE_SD, count, ids);

        for(unsigned i = 0; i < count; i++)
        {
            if(checkHigh(ids[i]) && !isBlacklisted(ids[i]))
            {
                titleData newTitle;
                if(newTitle.init(ids[i], MEDIATYPE_SD) && newTitle.hasSaveData())
                    titles.push_back(newTitle);
            }
        }
        delete[] ids;

        //Load NAND too now
        AM_GetTitleCount(MEDIATYPE_NAND, &count);

        ids = new uint64_t[count];
        AM_GetTitleList(NULL, MEDIATYPE_NAND, count, ids);
        for(unsigned i = 0; i < count; i++)
        {
            titleData newNandTitle;
            if(newNandTitle.init(ids[i], MEDIATYPE_NAND) && newNandTitle.hasSaveData() && !newNandTitle.getTitle().empty())
                titles.push_back(newNandTitle);
        }
        delete[] ids;

        t->status->setStatus("Writing cache...");
        createCache(titles, titlePath);
    }

    //Sort sort of like Switch
    //I don't like making copies but eh
    for(unsigned i = 0; i < titles.size(); i++)
    {
        data::titleSaveTypes tmp = titles[i].getSaveTypes();

        if(tmp.hasUser)
            usrSaveTitles.push_back(titles[i]);

        if(tmp.hasExt)
            extDataTitles.push_back(titles[i]);

        if(tmp.hasSys)
            sysDataTitles.push_back(titles[i]);

        if(tmp.hasBoss)
            bossDataTitles.push_back(titles[i]);
    }

    std::sort(usrSaveTitles.begin(), usrSaveTitles.end(), sortTitles);
    std::sort(extDataTitles.begin(), extDataTitles.end(), sortTitles);
    std::sort(sysDataTitles.begin(), sysDataTitles.end(), sortTitles);
    std::sort(bossDataTitles.begin(), bossDataTitles.end(), sortTitles);

    t->finished = true;
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
    std::sort(data::usrSaveTitles.begin(), data::usrSaveTitles.end(), sortTitles);
    std::sort(data::extDataTitles.begin(), data::extDataTitles.end(), sortTitles);
    std::sort(data::bossDataTitles.begin(), data::bossDataTitles.end(), sortTitles);
    std::sort(data::sysDataTitles.begin(), data::sysDataTitles.end(), sortTitles);
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

    std::sort(data::usrSaveTitles.begin(), data::usrSaveTitles.end(), sortTitles);
    std::sort(data::extDataTitles.begin(), data::extDataTitles.end(), sortTitles);
    std::sort(data::bossDataTitles.begin(), data::bossDataTitles.end(), sortTitles);
    std::sort(data::sysDataTitles.begin(), data::sysDataTitles.end(), sortTitles);
}

C2D_Image data::readIconFromSMDH(smdh_s *smdh)
{
    return (C2D_Image){loadIcon(smdh), &gfx::iconSubTex};
}

void data::createCache(std::vector<titleData>& vect, const std::string& path)
{
    //JIC
    fs::fdelete(path);
    fs::fsfile cache(fs::getSDMCArch(), path, FS_OPEN_CREATE | FS_OPEN_WRITE);

    //Buffer to compress icons
    size_t iconCmpSize = 0;
    uint8_t *iconOut = new uint8_t[ICON_BUFF_SIZE];

    uint16_t countOut = vect.size();
    cache.write(&countOut, sizeof(uint16_t));
    cache.putByte(0x05);

    for(auto t : vect)
    {
        uint16_t titleLength = t.getTitle().size();
        cache.write(&titleLength, sizeof(uint16_t));
        cache.write(t.getTitle().data(), titleLength * sizeof(char16_t));

        uint16_t publisherLength = t.getPublisher().size();
        cache.write(&publisherLength, sizeof(uint16_t));
        cache.write(t.getPublisher().data(), publisherLength * sizeof(char16_t));

        uint8_t prodLength = t.getProdCode().size();
        cache.write(&prodLength, sizeof(uint8_t));
        cache.write(t.getProdCode().c_str(), prodLength);

        uint64_t idOut = t.getID();
        cache.write(&idOut, sizeof(uint64_t));

        data::titleSaveTypes tmp = t.getSaveTypes();
        cache.write(&tmp, sizeof(data::titleSaveTypes));

        iconCmpSize = ICON_BUFF_SIZE;
        compress(iconOut, (uLongf *)&iconCmpSize, t.getIconData(), ICON_BUFF_SIZE);
        cache.write(&iconCmpSize, sizeof(size_t));
        cache.write(iconOut, iconCmpSize);
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

    if(rev != 5)
        return false;

    uint16_t count = 0;
    cache.read(&count, sizeof(uint16_t));
    cache.getByte();

    uint8_t *readBuff = new uint8_t[ICON_BUFF_SIZE];

    titles.reserve(count);

    for(unsigned i = 0; i < count; i++)
    {
        titleData newData;

        uint16_t titleLength = 0;
        char16_t title[0x40];
        memset(title, 0x00, 0x40 * sizeof(char16_t));
        cache.read(&titleLength, sizeof(char16_t));
        cache.read(title, titleLength * sizeof(char16_t));

        uint16_t pubLength = 0;
        char16_t pub[0x40];
        memset(pub, 0x00, 0x40 * sizeof(char16_t));
        cache.read(&pubLength, sizeof(uint16_t));
        cache.read(pub, pubLength * sizeof(char16_t));

        uint8_t prodLength = 0;
        char prodCode[16];
        memset(prodCode, 0x00, 16);
        cache.read(&prodLength, sizeof(uint8_t));
        cache.read(prodCode, prodLength);

        uint64_t newID = 0;
        cache.read(&newID, sizeof(uint64_t));

        data::titleSaveTypes tmp;
        cache.read(&tmp, sizeof(data::titleSaveTypes));

        size_t iconSize = 0;
        memset(readBuff, 0x00, ICON_BUFF_SIZE);
        cache.read(&iconSize, sizeof(size_t));
        cache.read(readBuff, iconSize);

        C3D_Tex *icon = new C3D_Tex;
        if(C3D_TexInit(icon, 64, 64, GPU_RGB565))
        {
            uLongf sz = ICON_BUFF_SIZE;
            uncompress((uint8_t *)icon->data, &sz, readBuff, iconSize);
            newData.assignIcon(icon);
        }
        newData.initFromCache(newID, title, pub, prodCode, tmp, nand ? MEDIATYPE_NAND : MEDIATYPE_SD);
        vect.push_back(newData);
    }

    delete[] readBuff;

    return true;
}

void data::datDrawTop()
{
    ui::drawUIBar("Loading...", ui::SCREEN_TOP, true);
}

void data::datDrawBot()
{
    ui::drawUIBar("", ui::SCREEN_BOT, false);
}

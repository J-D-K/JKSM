#include <3ds.h>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include "util.h"
#include "data.h"
#include "fs.h"
#include "ui.h"
#include "gfx.h"

static const char16_t verboten[] = { L'.', L',', L'/', L'\\', L'<', L'>', L':', L'"', L'|', L'?', L'*' };

static uint32_t extdataRedirect(const uint32_t& low)
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
    //Ultra Moon
    else if(low == 0x001B5100)
        return 0x00001B50;
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

std::vector<uint64_t> blacklist;

bool isBlacklisted(const uint64_t& id)
{
    for(unsigned i = 0; i < blacklist.size(); i++)
    {
        if(id == blacklist[i])
            return true;
    }

    return false;
}

namespace data
{
    std::vector<titleData> titles;
    std::vector<titleData> nand;
    std::vector<uint32_t> filterIds;

    bool haxMode = false;

    titleData curData;

    bool isVerboten(const char16_t& c)
    {
        for(unsigned i = 0; i < 11; i++)
        {
            if(c == verboten[i])
                return true;
        }

        return false;
    }

    std::u16string safeTitle(const std::u16string& s)
    {
        std::u16string ret;
        for(unsigned i = 0; i < s.length(); i++)
        {
            if(isVerboten(s[i]))
                ret += L' ';
            else
                ret += s[i];
        }

        //Erase space if last char
        if(ret[ret.length() - 1] == L' ')
            ret.erase(ret.length() - 1, ret.length());

        return ret;
    }

    bool titleData::init(const uint64_t& _id, const FS_MediaType& mt)
    {
        m = mt;
        id = _id;

        low = (uint32_t)id;
        high = (uint32_t)(id >> 32);
        unique = (low >> 8);
        extdata = extdataRedirect(low);

        smdh_s *smdh = loadSMDH(low, high, m);
        if(smdh == NULL)
            return false;

        title.assign((char16_t *)(smdh->applicationTitles[1].shortDescription));
        titleSafe.assign(safeTitle(title));

        char tmp[16];
        AM_GetTitleProductCode(m, id, tmp);
        prodCode = tmp;

        delete smdh;

        return true;
    }

    bool titleData::initFromCache(const uint64_t& _id, const std::u16string& _title, const std::string& code, const uint8_t& mt)
    {
        id = _id;
        low = (uint32_t)id;
        high = (uint32_t)(id >> 32);
        unique = (low >> 8);
        extdata = extdataRedirect(low);
        m = (FS_MediaType)mt;


        title.assign(_title);
        titleSafe.assign(safeTitle(title));
        prodCode.assign(code);

        return true;
    }

    bool titleData::isOpenable()
    {
        bool ret = false;

        if(getMedia() == MEDIATYPE_GAME_CARD || getMedia() == MEDIATYPE_SD)
        {
            ret = fs::openArchive(*this, ARCHIVE_USER_SAVEDATA);
            if(!ret)
                ret = fs::openArchive(*this, ARCHIVE_EXTDATA);
        }

        if(getMedia() == MEDIATYPE_NAND)
        {
            ret = fs::openArchive(*this, ARCHIVE_SYSTEM_SAVEDATA);

            if(!ret)
                ret = fs::openArchive(*this, ARCHIVE_EXTDATA);

            if(!ret)
                ret = fs::openArchive(*this, ARCHIVE_BOSS_EXTDATA);

        }

        FSUSER_CloseArchive(fs::getSaveArch());

        return ret;
    }

    void titleData::drawInfo(unsigned x, unsigned y)
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
    }

    void cartCheck()
    {
        bool ins = false;
        FSUSER_CardSlotIsInserted(&ins);

        if(titles[0].getMedia() != MEDIATYPE_GAME_CARD && ins)
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
            titles.erase(titles.begin(), titles.begin() + 1);
            ui::loadTitleMenu();
        }
    }

    smdh_s *loadSMDH(const uint32_t& low, const uint32_t& high, const uint8_t& media)
    {
        Handle handle;

        uint32_t archPath[] = {low, high, media, 0};
        static const uint32_t filePath[] = {0x0, 0x0, 0x2, 0x6E6F6369, 0x0};

        FS_Path binArchPath = {PATH_BINARY, 0x10, archPath};
        FS_Path binFilePath = {PATH_BINARY, 0x14, filePath};

        Result res = FSUSER_OpenFileDirectly(&handle, ARCHIVE_SAVEDATA_AND_CONTENT, binArchPath, binFilePath, FS_OPEN_READ, 0);
        if(res == 0)
        {
            uint32_t read = 0;
            smdh_s *ret = new smdh_s;
            FSFILE_Read(handle, &read, 0, ret, sizeof(smdh_s));
            FSFILE_Close(handle);

            return ret;
        }

        return NULL;
    }

    struct
    {
        bool operator()(titleData& a, titleData& b)
        {
            for(unsigned i = 0; i < a.getTitle().length(); i++)
            {
                int aChar = tolower(a.getTitle()[i]), bChar = tolower(b.getTitle()[i]);
                if(aChar != bChar)
                    return aChar < bChar;
            }

            return false;
        }
    } sortTitles;

    bool checkHigh(const uint64_t& id)
    {
        uint32_t high = (uint32_t)(id >> 32);

        return (high == 0x00040000 || high == 0x00040002);
    }

    void loadTitles()
    {
        titles.clear();
        blacklist.clear();
        loadBlacklist();

        if(!readCache(titles, "/JKSV/titles", false))
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
                    {
                        titles.push_back(newTitle);
                    }
                }
                else

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

            createCache(titles, "/JKSV/titles");
        }
        curData = titles[0];
    }

    void loadNand()
    {
        nand.clear();

        if(!readCache(nand, "/JKSV/nand", true))
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
                {
                    nand.push_back(newNandTitle);
                }
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

            createCache(nand, "/JKSV/nand");
        }
    }

    void loadBlacklist()
    {
        if(util::fexists("/JKSV/blacklist.txt"))
        {
            std::fstream bl("/JKSV/blacklist.txt", std::ios::in);

            std::string line;
            while(std::getline(bl, line))
            {
                if(line[0] == '#' || line[0] == '\n')
                    continue;

                uint64_t pushID = std::strtoull(line.c_str(), NULL, 16);

                blacklist.push_back(pushID);
            }
            bl.close();
        }
    }

    void blacklistAdd(titleData& t)
    {
        if(t.getMedia() == MEDIATYPE_GAME_CARD)
            return;

        std::fstream bl("/JKSV/blacklist.txt", std::ios::app);

        std::string titleLine = "#" + util::toUtf8(t.getTitle()) + "\n";
        char idLine[32];
        sprintf(idLine, "0x%016llX\n", t.getID());

        bl.write(titleLine.c_str(), titleLine.length());
        bl.write(idLine, std::strlen(idLine));
        bl.close();

        //Remove it
        for(unsigned i = 0; i < titles.size(); i++)
        {
            if(titles[i].getID() == t.getID())
            {
                titles.erase(titles.begin() + i);
                break;
            }
        }

        //Erase cart if it's there
        if(titles[0].getMedia() == MEDIATYPE_GAME_CARD)
            titles.erase(titles.begin());

        //Recreate cache with title missing now
        createCache(titles, "/JKSV/titles");

        //Reinit title menu
        ui::loadTitleMenu();
    }

    void createCache(std::vector<titleData>& t, const std::string& path)
    {
        //JIC
        std::remove(path.c_str());

        std::fstream cache(path, std::ios::out | std::ios::binary);

        uint16_t countOut = t.size();
        cache.write((char *)&countOut, sizeof(uint16_t));
        cache.put(0x02);

        for(unsigned i = 0; i < t.size(); i++)
        {
            char16_t titleOut[0x40];
            std::memset(titleOut, 0, 0x40 * 2);
            std::memcpy((void *)titleOut, (void *)t[i].getTitle().data(), t[i].getTitle().length() * 2);
            cache.write((char *)titleOut, 0x40 * 2);
            cache.put(0x00);

            char prodOut[16];
            std::memset(prodOut, 0, 16);
            std::memcpy(prodOut, t[i].getProdCode().data(), 16);
            cache.write(prodOut, 16);
            cache.put(0x00);

            uint64_t idOut = t[i].getID();
            cache.write((char *)&idOut, sizeof(uint64_t));
            cache.put(0x00);
        }

        cache.close();
    }

    bool readCache(std::vector<titleData>& t, const std::string& path, bool nand)
    {
        if(!util::fexists(path))
            return false;

        std::fstream cache(path, std::ios::in | std::ios::binary);
        //Check revision
        uint8_t rev = 0;
        cache.seekg(2, cache.beg);
        rev = cache.get();
        cache.seekg(0, cache.beg);

        if(rev != 2)
        {
            cache.close();
            return false;
        }

        uint16_t count = 0;
        cache.read((char *)&count, sizeof(uint16_t));

        cache.seekg(1, cache.cur);

        for(unsigned i = 0; i < count; i++)
        {
            titleData newData;

            char16_t title[0x40];
            cache.read((char *)title, sizeof(char16_t) * 0x40);
            cache.seekg(1, cache.cur);

            char prodCode[16];
            cache.read(prodCode, 16);
            cache.seekg(1, cache.cur);

            uint64_t newID = 0;
            cache.read((char *)&newID, sizeof(uint64_t));
            cache.seekg(1, cache.cur);

            newData.initFromCache(newID, title, prodCode, nand ? MEDIATYPE_NAND : MEDIATYPE_SD);
            t.push_back(newData);
        }

        cache.close();

        return true;
    }

    void haxDataInit()
    {
        uint64_t id;
        APT_GetProgramID(&id);

        fs::fsStartSession();
        FS_MediaType getMedia;
        FSUSER_GetMediaType(&getMedia);
        fs::fsEndSession();

        titleData haxData;
        haxData.init(id, getMedia);

        curData = haxData;
    }
}

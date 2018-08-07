#include <3ds.h>
#include <cstring>
#include <ctime>
#include <fstream>

#include "util.h"
#include "fs.h"
#include "gfx.h"

#include "ui.h"

namespace util
{
    std::string toUtf8(const std::u16string& conv)
    {
        uint8_t tmp[1024];
        std::memset(tmp, 0, 1024);
        utf16_to_utf8(tmp, (uint16_t *)conv.data(), 1024);
        return std::string((char *)tmp);
    }

    std::u16string toUtf16(const std::string& conv)
    {
        char16_t tmp[1024];
        std::memset(tmp, 0, 1024 * sizeof(char16_t));
        utf8_to_utf16((uint16_t *)tmp, (uint8_t *)conv.data(), 1024);
        return std::u16string(tmp);
    }

    std::u16string createPath(data::titleData& dat, const uint32_t& mode)
    {
        std::u16string ret;
        switch(mode)
        {
            case ARCHIVE_USER_SAVEDATA:
            case ARCHIVE_SAVEDATA:
                ret = toUtf16("/JKSV/Saves/") + dat.getTitleSafe() + toUtf16("/");
                break;

            case ARCHIVE_SYSTEM_SAVEDATA:
                ret = toUtf16("/JKSV/SysSave/") + dat.getTitleSafe() + toUtf16("/");
                break;

            case ARCHIVE_EXTDATA:
                ret = toUtf16("/JKSV/ExtData/") + dat.getTitleSafe() + toUtf16("/");
                break;

            case ARCHIVE_BOSS_EXTDATA:
                ret = toUtf16("/JKSV/Boss/") + dat.getTitleSafe() + toUtf16("/");
                break;

            case ARCHIVE_SHARED_EXTDATA:
                char tmp[16];
                sprintf(tmp, "%08X", (unsigned)dat.getExtData());
                ret = toUtf16("/JKSV/Shared/") + toUtf16(tmp) + toUtf16("/");
                break;
        }
        return ret;
    }

    std::string getString(const std::string& hint, bool def)
    {
        SwkbdState state;
        char input[128];

        swkbdInit(&state, SWKBD_TYPE_NORMAL, 2, 128);
        swkbdSetHintText(&state, hint.c_str());

        if(def)//If default text/date suggestion
        {
            //FUTURE PROOFING DIS TIME. Just grab current year
            std::string year = std::string(util::getDateString(DATE_FMT_YMD)).substr(0, 4);

            char tmpDate[128];
            sprintf(tmpDate, "%s", util::getDateString(DATE_FMT_YMD).c_str());
            swkbdSetInitialText(&state, tmpDate);

            swkbdSetFeatures(&state, SWKBD_PREDICTIVE_INPUT);
            SwkbdDictWord date[2];
            swkbdSetDictWord(&date[0], year.c_str(), util::getDateString(DATE_FMT_YMD).c_str());
            swkbdSetDictWord(&date[1], year.c_str(), util::getDateString(DATE_FMT_YDM).c_str());
            swkbdSetDictionary(&state, date, 2);
        }

        swkbdInputText(&state, input, 128);

        return std::string(input);
    }

    int getInt(const std::string& hint, const int& init, const int& max)
    {
        int ret = 0;
        SwkbdState keyState;
        char in[8];

        swkbdInit(&keyState, SWKBD_TYPE_NUMPAD, 2, 8);
        swkbdSetHintText(&keyState, hint.c_str());
        if(init != -1)
        {
            sprintf(in, "%i", init);
            swkbdSetInitialText(&keyState, in);
        }

        SwkbdButton pressed = swkbdInputText(&keyState, in, 8);

        if(pressed == SWKBD_BUTTON_LEFT)
            ret = -1;
        else
        {
            ret = std::strtol(in, NULL, 10);
            if(ret > max)
                ret = max;
        }

        return ret;
    }

    std::string getDateString(const int& fmt)
    {
        char tmp[128];

        time_t rawTime;
        time(&rawTime);
        tm *local = localtime(&rawTime);
        switch(fmt)
        {
            case DATE_FMT_YMD:
                sprintf(tmp, "%04d-%02d-%02d_%02d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
                break;

            case DATE_FMT_YDM:
                sprintf(tmp, "%04d-%02d-%02d_%02d-%02d-%02d", local->tm_year + 1900, local->tm_mday, local->tm_mon + 1, local->tm_hour, local->tm_min, local->tm_sec);
                break;
        }

        return std::string(tmp);
    }

    std::string getWrappedString(const std::string& s, const unsigned& maxWidth)
    {

        if(gfx::getTextWidth(s) < maxWidth)
            return s;

        std::string ret = "", tmp = "";
        unsigned first = 0, lastSpace = 0;

        for(unsigned i = 0; i < s.length(); i++)
        {
            tmp += s[i];

            if(s[i] == ' ' || s[i] == '/')
                lastSpace = i;

            if(gfx::getTextWidth(tmp) >= maxWidth)
            {
                tmp.assign(s, first, (lastSpace + 1) - first);

                ret += tmp + "\n";

                first = lastSpace + 1;
                i = lastSpace;

                tmp.clear();
            }
        }
        if(!tmp.empty())
            ret += tmp;

        return ret;
    }

    void removeLastDirFromString(std::u16string& s)
    {
        unsigned last = s.find_last_of(L'/', s.length() - 2);
        s.erase(last + 1, s.length());
    }

    void createTitleDir(data::titleData& dat, const uint32_t& mode)
    {
        std::u16string cr;
        switch(mode)
        {
            case ARCHIVE_USER_SAVEDATA:
            case ARCHIVE_SAVEDATA:
                cr = toUtf16("/JKSV/Saves/") + dat.getTitleSafe();
                break;

            case ARCHIVE_SYSTEM_SAVEDATA:
                cr = toUtf16("/JKSV/SysSave/") + dat.getTitleSafe();
                break;

            case ARCHIVE_EXTDATA:
                cr = toUtf16("/JKSV/ExtData/") + dat.getTitleSafe();
                break;

            case ARCHIVE_BOSS_EXTDATA:
                cr = toUtf16("/JKSV/Boss/") + dat.getTitleSafe();
                break;

            case ARCHIVE_SHARED_EXTDATA:
                char tmp[16];
                sprintf(tmp, "%08X", (unsigned)dat.getExtData());
                cr = toUtf16("/JKSV/Shared/") + toUtf16(tmp);
                break;
        }
        FSUSER_CreateDirectory(fs::getSDMCArch(), fsMakePath(PATH_UTF16, cr.data()), 0);
    }

    void copyDirlistToMenu(fs::dirList& d, ui::menu& m)
    {
        m.reset();

        m.addOpt(".", 320);
        m.addOpt("..", 320);

        for(unsigned i = 0; i < d.getCount(); i++)
        {
            if(d.isDir(i))
                m.addOpt("D " + toUtf8(d.getItem(i)), 320);
            else
                m.addOpt("F " + toUtf8(d.getItem(i)), 320);
        }
    }

    void setPC()
    {
        data::titleData tmp;
        tmp.setExtdata(0xF000000B);
        if(fs::openArchive(tmp, ARCHIVE_SHARED_EXTDATA))
        {
            fs::fsfile playCoin(fs::getSaveArch(), "/gamecoin.dat", FS_OPEN_READ | FS_OPEN_WRITE);

            int coinAmount = 0;
            playCoin.seek(0x4, fs::seek_beg);
            coinAmount = playCoin.getByte() | playCoin.getByte() << 8;

            coinAmount = getInt("Enter a number between 0 and 300", coinAmount, 300);
            if(coinAmount != -1)
            {
                playCoin.seek(-2, fs::seek_cur);
                playCoin.putByte(coinAmount);
                playCoin.putByte(coinAmount >> 8);
            }

            fs::closeSaveArch();
        }
    }

    bool touchPressed(const touchPosition& p)
    {
        return p.px != 0 && p.py != 0;
    }

    bool fexists(const std::string& path)
    {
        std::fstream test(path, std::ios::in);
        if(test.is_open())
        {
            test.close();

            return true;
        }

        return false;
    }
}

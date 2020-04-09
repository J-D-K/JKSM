#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <3ds.h>
#include "data.h"
#include "fs.h"
#include "ui.h"

namespace util
{
    enum datFmt
    {
        DATE_FMT_YMD,
        DATE_FMT_YDM
    };

    std::string toUtf8(const std::u16string& conv);
    std::u16string toUtf16(const std::string& conv);
    std::u16string createPath(data::titleData& dat, const uint32_t& mode);
    std::string getString(const std::string& hint, bool def);
    std::u16string safeString(const std::u16string& s);
    int getInt(const std::string& hint, const int& init, const int& max);
    std::string getDateString(const int& fmt);

    std::string getWrappedString(const std::string& s, const unsigned& maxWidth);
    void removeLastDirFromString(std::u16string& s);

    void createTitleDir(data::titleData& dat, const uint32_t& mode);

    void copyDirlistToMenu(fs::dirList& d, ui::menu& m);

    void setPC();

    bool touchPressed(const touchPosition& p);

    bool fexists(const std::string& path);
}

#endif

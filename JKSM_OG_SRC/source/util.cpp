#include <3ds.h>
#include <stdio.h>
#include <string>
#include <string.h>

#include "global.h"
#include "util.h"
#include "titledata.h"
#include "file.h"
#include "ui.h"
#include "smdh.h"
#include "archive.h"
#include "menu.h"

static Handle fsHandle;

std::u32string tou32(const std::u16string t)
{
    char32_t tmp[256];
    memset(tmp, 0, 256);

    utf16_to_utf32((uint32_t *)tmp, (uint16_t *)t.data(), 256);

    return std::u32string(tmp);
}


std::u16string tou16(const char *t)
{
    char16_t tmp[256];
    memset(tmp, 0, 256);

    utf8_to_utf16((uint16_t *)tmp, (uint8_t *)t, 256);

    return std::u16string(tmp);
}

std::string toString(const std::u16string t)
{
    std::string ret;

    for(unsigned i = 0; i < t.length(); i++)
        ret += t[i];

    return ret;
}

std::u32string modeText(int mode)
{
    switch(mode)
    {
        case MODE_SAVE:
            return U" : Save";
            break;
        case MODE_EXTDATA:
            return U" : ExtData";
            break;
        case MODE_BOSS:
            return U" : Boss ExtData";
            break;
        case MODE_SYSSAVE:
            return U" : System Save";
            break;
        case MODE_SHARED:
            return U" : Shared ExtData";
            break;
        default:
            return U"";
            break;
    }
}

void writeErrorToBuff(u8 *buff, size_t bSize, unsigned error)
{
    char bytes[16];
    sprintf(bytes, "****%08X****", error);

    for(unsigned i = 0, cByte = 0; i < bSize; i++, cByte++)
    {
        if(cByte > 15)
            cByte = 0;
        buff[i] = bytes[cByte];
    }
}

void createTitleDir(const titleData t, int mode)
{
    std::u16string create = getPath(mode) + t.nameSafe;

    FSUSER_CreateDirectory(sdArch, fsMakePath(PATH_UTF16, create.data()), 0);
}

void renameU16(std::u16string oldName, std::u16string newName)
{
    FSUSER_RenameDirectory(sdArch, fsMakePath(PATH_UTF16, oldName.data()), sdArch, fsMakePath(PATH_UTF16, newName.data()));
}

void renameDir(const titleData t)
{
    std::u16string oldName, oldPath, newPath;
    if(hbl)
    {
        //get old one with '_'s
        oldName = safeStringOld(t.name);

        //Rename save dirs
        oldPath = getPath(MODE_SAVE) + oldName;
        newPath = getPath(MODE_SAVE) + t.nameSafe;
        renameU16(oldPath, newPath);

        //rename extdat
        oldPath = getPath(MODE_EXTDATA) + oldName;
        newPath = getPath(MODE_EXTDATA) + t.nameSafe;
        renameU16(oldPath, newPath);
    }
    else if(t.media == MEDIATYPE_SD || t.media == MEDIATYPE_GAME_CARD)
    {
        oldName = safeStringOld(t.name);

        oldPath = getPath(MODE_SAVE) + oldName;
        newPath = getPath(MODE_SAVE) + t.nameSafe;
        renameU16(oldPath, newPath);

        oldPath = getPath(MODE_EXTDATA) + oldName;
        newPath = getPath(MODE_EXTDATA) + t.nameSafe;
        renameU16(oldPath, newPath);
    }
    else if(t.media == MEDIATYPE_NAND)
    {
        oldName = safeStringOld(t.name);

        oldPath = getPath(MODE_SYSSAVE) + oldName;
        newPath = getPath(MODE_SYSSAVE) + t.nameSafe;
        renameU16(oldPath, newPath);

        oldPath = getPath(MODE_EXTDATA) + oldName;
        newPath = getPath(MODE_EXTDATA) + t.nameSafe;
        renameU16(oldPath, newPath);

        oldPath = getPath(MODE_BOSS) + oldName;
        newPath = getPath(MODE_BOSS) + t.nameSafe;
        renameU16(oldPath, newPath);
    }
}

bool deleteSV(const titleData t)
{
    u64 in = ((u64)SECUREVALUE_SLOT_SD << 32) | (t.unique << 8);
    u8 out;

    Result res = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &in, 8, &out, 1);
    if(res)
    {
        showError("Failed to delete secure value", (u32)res);
        return false;
    }

    return true;
}

std::u16string getPath(int mode)
{
    switch(mode)
    {
        case MODE_SAVE:
            return tou16("/3ds/data/JKSM/Saves/");
            break;
        case MODE_EXTDATA:
            return tou16("/3ds/data/JKSM/ExtData/");
            break;
        case MODE_BOSS:
            return tou16("/3ds/data/JKSM/Boss/");
            break;
        case MODE_SYSSAVE:
            return tou16("/3ds/data/JKSM/SysSave/");
            break;
        case MODE_SHARED:
            return tou16("/3ds/data/JKSM/Shared/");
            break;
        default:
            return tou16("/3ds/data/JKSM/");
            break;
    }
}

bool runningUnder()
{
    u64 id;
    APT_GetProgramID(&id);

    return id != 0x0004000002c23200;
}

void deleteExtdata(const titleData dat)
{
    FS_ExtSaveDataInfo del = {MEDIATYPE_SD, 0, 0, dat.extdata, 0};

    Result res = FSUSER_DeleteExtSaveData(del);
    if(res)
        showError("Error deleting Extra Data", (u32)res);
    else
        showMessage("Extra Data deleted!", "Success");
}

void createExtData(const titleData dat)
{
    FS_ExtSaveDataInfo create = {MEDIATYPE_SD, 0, 0, dat.extdata, 0};
    smdh_s *tempSmdh = loadSMDH(dat.low, dat.high, dat.media);

    //100 should be enough, right?
    Result res;
    if(tempSmdh == NULL)
    {
        u8 *emptySmdh = new u8[sizeof(smdh_s)];
        memset(emptySmdh, 0, sizeof(smdh_s));
        res = FSUSER_CreateExtSaveData(create, 100, 100, 0x10000000, sizeof(smdh_s), emptySmdh);
        delete[] emptySmdh;
    }
    else
        res = FSUSER_CreateExtSaveData(create, 100, 100, 0x10000000, sizeof(smdh_s), (u8 *)tempSmdh);
    if(res)
    {
        showError("Error creating Extra Data", (u32)res);
    }
    else
        showMessage("ExtData created!", "Success!");

    delete tempSmdh;
}

void evenString(std::string *test)
{
    if(test->length() % 2 == 0)
        test->append(" ");
}

//Just returns whether or not the touch screen is pressed anywhere.
bool touchPressed(touchPosition p)
{
    return (p.px > 0 || p.py > 0);
}

bool modeExtdata(int mode)
{
    return (mode == MODE_EXTDATA || mode == MODE_BOSS || mode == MODE_SHARED);
}

bool fexists(const char *path)
{
    FILE *test = fopen(path, "r");
    if(test == NULL)
        return false;

    fclose(test);

    return true;
}

void fsStart()
{
    srvGetServiceHandleDirect(&fsHandle, "fs:USER");
    FSUSER_Initialize(fsHandle);
    fsUseSession(fsHandle);
}

void fsEnd()
{
    fsEndUseSession();
}

void fsCommitData(FS_Archive arch)
{
    FSUSER_ControlArchive(arch, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
}

//I seriously can't remember why I put space in there. I don't like it anymore.
const char16_t oldVerboten[] = { L' ', L'.', L',', L'/', L'\\', L'<', L'>', L':', L'"', L'|', L'?', L'*'};//12
const char16_t newVerboten[] = { L'.', L',', L'/', L'\\', L'<', L'>', L':', L'"', L'|', L'?', L'*'};

bool isVerbotenOld(char16_t d)
{
    for(int i = 0; i < 12; i++)
    {
        if(d == oldVerboten[i])
            return true;
    }

    return false;
}

bool isVerboten(char16_t d)
{
    for(int i = 0; i < 11; i++)
    {
        if(d == newVerboten[i])
            return true;
    }

    return false;
}

std::u16string safeStringOld(const std::u16string s)
{
    std::u16string ret;
    for(unsigned i = 0; i < s.length(); i++)
    {
        if(isVerbotenOld(s[i]))
            ret += L'_';
        else
            ret += s[i];
    }
    return ret;
}

std::u16string safeString(const std::u16string s)
{
    std::u16string ret;
    for(unsigned i = 0; i < s.length(); i++)
    {
        if(isVerboten(s[i]))
            ret += ' ';
        else
            ret += s[i];
    }

    int i;
    for(i = ret.length() - 1; i > 0; i--)
    {
        if(ret[i] != L' ')
            break;
    }

    ret.erase(i + 1, ret.length() - i);

    return ret;
}

void prepareMenus()
{
    prepMain();
    prepBackMenu();
    prepSaveMenu();
    prepExtMenu();
    prepNandBackup();
    prepSharedMenu();
    prepSharedBackMenu();
    prepExtras();
    if(devMode)
        prepDevMenu();
}

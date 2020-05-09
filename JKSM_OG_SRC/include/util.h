#ifndef UTIL_H
#define UTIL_H

#include <sf2d.h>
#include <string>

#include "titledata.h"

//return converted strings using ctrulib's utfX_to_utfX
std::u32string tou32(const std::u16string t);
std::u16string tou16(const char *t);
std::string toString(const std::u16string t);

//Returns the " : XXXX Data"
std::u32string modeText(int mode);

//Returns utf16 string with forbidden chars removed
std::u16string safeString(const std::u16string s);
std::u16string safeStringOld(const std::u16string s);

//Writes the error given over the buffer for broken data
void writeErrorToBuff(u8 *buff, size_t bSize, unsigned error);

//Creates the directory for data
void createTitleDir(const titleData t, int mode);
void renameDir(const titleData t);

//Deletes secure value
bool deleteSV(const titleData t);

//Returns path to folders to make it easier to change if needed
std::u16string getPath(int mode);

//Guess what these do.
void deleteExtdata(const titleData dat);
void createExtData(const titleData dat);

//returns if dealing with extdata
bool modeExtdata(int mode);

//Makes a string odd if it's even.
void evenString(std::string *test);

//This detects if running under something as 3dsx
bool runningUnder();

//returns if touchscreen is still being pressed anywhere
bool touchPressed(touchPosition p);

bool fexists(const char *path);

//Starts and ends fs sessions for HBL mode
void fsStart();
void fsEnd();
void fsCommitData(FS_Archive arch);
Result FS_GetMediaType(FS_MediaType *m);

//Sets up menus
void prepareMenus();

sf2d_texture *createTexMem(struct img);
#endif // UTIL_H

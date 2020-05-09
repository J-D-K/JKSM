#ifndef GLOBAL_H
#define GLOBAL_H

#include <sf2d.h>
#include <sftd.h>

#include <string>

#include "titledata.h"

//this is for backup/restore modes
#define MODE_SAVE 0
#define MODE_EXTDATA 1
#define MODE_SYSSAVE 2
#define MODE_BOSS 3
#define MODE_SHARED 4

#define BUILD_DATE std::u32string(U"2-10-2017")

extern int state, prevState;
extern u8 sysLanguage;
extern titleData *curTitle;

enum states
{
    STATE_MAINMENU,
    STATE_TITLEMENU,
    STATE_BACKUPMENU,
    STATE_SAVEMENU,
    STATE_EXTMENU,
    STATE_NANDSELECT,
    STATE_NANDBACKUP,
    STATE_EXTRAS,
    STATE_SHARED,
    STATE_SHAREDBACKUP,
    STATE_DEVMENU
};

void handleState();

//buffer size
extern const unsigned buff_size;

//This is the font used to print everything
extern sftd_font *font;

//SDMC Archive. Seems to write faster using this. Might just be me though
extern FS_Archive sdArch;

extern bool devMode, hbl, kill;
//config
extern bool centered, autoBack, useLang;

//Allows app to be killed by hitting start
void killApp(u32 up);

//got tired of black, gray, and green all the time
extern u8 clearColor[3];
extern u8 selColor[3];
extern u8 unSelColor[3];

void mainMenu();

#endif

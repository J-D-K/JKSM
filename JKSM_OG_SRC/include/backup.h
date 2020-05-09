#ifndef BACKUP_H
#define BACKUP_H

#include "titledata.h"
#include "menu.h"

/*
dat = titeData containing info for title
arch = opened FS_Archive
mode = where to export data to
auto = whether or not to just grab date or get user input
*/
bool backupData(const titleData dat, FS_Archive arch, int mode, bool autoName);
void autoBackup(menu m);
#endif // BACKUP_H

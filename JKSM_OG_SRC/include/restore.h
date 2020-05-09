#ifndef RESTORE_H
#define RESTORE_H

#include "titledata.h"
#include "menu.h"

bool restoreData(const titleData dat, FS_Archive arch, int mode);
bool restoreDataSDPath(const titleData dat, FS_Archive arch, int mode);
void autoRestore(menu m);

#endif // RESTORE_H

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "titledata.h"

/*
These open the 3DS's archives
return true or false
out is the archive opened if it didn't fail
showError is whether or not to show the popup if it fails
*/
bool openSaveArch(FS_Archive *out, const titleData dat, bool show);
bool openSaveArch3dsx(FS_Archive *out);
bool openExtdata(FS_Archive *out, const titleData dat, bool show);
bool openSharedExt(FS_Archive *out, u32 id);
bool openSysSave(FS_Archive *out, const titleData dat);
bool openBossExt(FS_Archive *out, const titleData dat);

#endif // ARCHIVE_H

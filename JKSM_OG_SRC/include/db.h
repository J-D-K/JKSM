#ifndef DB_H
#define DB_H

#include <stdio.h>
#include "titledata.h"

FILE *dbCreate(const char *path);
FILE *dbOpen(const char *path);

void dbWriteCount(FILE *db, u32 count, u8 rev);
u32 dbGetCount(FILE *db);
u8 dbGetRev(FILE *db);

void dbWriteData(FILE *db, const titleData w);
titleData dbGetData(FILE *db);

#endif // DB_G

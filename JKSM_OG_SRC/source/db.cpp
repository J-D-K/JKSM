#include <3ds.h>
#include <stdio.h>
#include <string>
#include <string.h>

#include "db.h"
#include "titledata.h"

//opens, creates and returns a file for writing to.
FILE *dbCreate(const char *path)
{
    FILE *db = fopen(path, "wb");
    return db;
}

//opens and returns file for reading
FILE *dbOpen(const char *path)
{
    FILE *db = fopen(path, "rb");
    return db;
}

//This is for writing the number of titles installed
void dbWriteCount(FILE *db, u32 count, u8 rev)
{
    fwrite(&count, sizeof(uint16_t), 1, db);

    fputc(rev, db);
}

//this gets the count from the beginning
//should be used first after opening for reading.
u32 dbGetCount(FILE *db)
{
    u32 ret = 0;
    fread(&ret, sizeof(uint16_t), 1, db);

    fgetc(db);

    return ret;
}

//This was a spacer byte, but I'll use it for this.
u8 dbGetRev(FILE *db)
{
    fseek(db, 2, SEEK_SET);
    u8 ret = fgetc(db);
    fseek(db, 0, SEEK_SET);

    return ret;
}

void copyu16(char16_t *out, std::u16string in)
{
    memset(out, 0, sizeof(char16_t) * 0x40);
    for(unsigned i = 0; i < in.length(); i++)
        out[i] = in.data()[i];
}

//writes needed data from titleData 'w' to file
void dbWriteData(FILE *db, const titleData w)
{
    //UTF16 title
    char16_t titleOut[0x40];
    copyu16(titleOut, w.name);
    fwrite(titleOut, sizeof(char16_t), 0x40, db);
    fputc(0, db);

    //ASCII product code
    char prodOut[16];
    memset(prodOut, 0, 16);
    sprintf(prodOut, "%s", w.prodCode.c_str());
    fwrite(prodOut, 1, 16, db);
    fputc(0, db);

    fwrite(&w.id, sizeof(u64), 1, db);

    fputc(0, db);
}

//reads and returns a titleData object from db
titleData dbGetData(FILE *db)
{
    titleData ret;

    char16_t title[0x40];
    fread(title, sizeof(char16_t), 0x40, db);
    ret.name.assign(title);

    fgetc(db);

    char prodCode[16];
    fread(prodCode, 1, 16, db);
    ret.prodCode.assign(prodCode);

    fgetc(db);

    fread(&ret.id, sizeof(u64), 1, db);

    //last 0x00
    fgetc(db);

    //this takes care of a few things needed
    ret.initId();

    return ret;
}

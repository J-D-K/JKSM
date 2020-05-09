#include <3ds.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "file.h"
#include "util.h"
#include "ui.h"

fsFile::fsFile(FS_Archive _arch, std::u16string _path, u32 openFlags)
{
    arch = _arch;
    error = FSUSER_OpenFile(&fileHandle, arch, fsMakePath(PATH_UTF16, _path.data()), openFlags, 0);
    if(error)
    {
        opened = false;
    }
    else
    {
        FSFILE_GetSize(fileHandle, &fileSize);
        opened = true;
    }
}

fsFile::fsFile(FS_Archive _arch, std::u16string _path, u32 openFlags, u64 createSize)
{
    arch = _arch;
    error = FSUSER_OpenFile(&fileHandle, arch, fsMakePath(PATH_UTF16, _path.data()), openFlags, 0);
    if(error)
    {
        error = FSUSER_CreateFile(arch, fsMakePath(PATH_UTF16, _path.data()), 0, createSize);
        if(error)
        {
            opened = false;
            return;
        }
        error = FSUSER_OpenFile(&fileHandle, arch, fsMakePath(PATH_UTF16, _path.data()), openFlags, 0);
        if(error)
            opened = false;
        else
            opened = true;
    }
}

fsFile::fsFile(FS_Archive _arch, const char *_path, u32 openFlags)
{
    arch = _arch;
    error = FSUSER_OpenFile(&fileHandle, arch, fsMakePath(PATH_ASCII, _path), openFlags, 0);
    if(error)
        opened = false;
    else
    {
        FSFILE_GetSize(fileHandle, &fileSize);
        opened = true;
    }
}


fsFile::fsFile(FS_Archive _arch, const char *_path, u32 openFlags, u64 createSize)
{
    arch = _arch;
    error = FSUSER_OpenFile(&fileHandle, arch, fsMakePath(PATH_ASCII, _path), openFlags, 0);
    if(error)
    {
        error = FSUSER_CreateFile(arch, fsMakePath(PATH_ASCII, _path), 0, createSize);
        if(error)
        {
            opened = false;
            return;
        }
        error = FSUSER_OpenFile(&fileHandle, arch, fsMakePath(PATH_ASCII, _path), openFlags, 0);
        if(error)
            opened = false;
        else
            opened = true;
    }
}

fsFile::fsFile(FS_ArchiveID _arch, FS_Path archPath, FS_Path filePath, u32 openFlags)
{
    error = FSUSER_OpenFileDirectly(&fileHandle, _arch, archPath, filePath, openFlags, 0);
    if(error == 0)
        opened = true;
    else
        opened = false;
}

fsFile::fsFile(FS_Archive _arch, FS_Path _path, u32 openFlags)
{
    error = FSUSER_OpenFile(&fileHandle, _arch, _path, openFlags, 0);
    if(error)
        opened = false;
    else
        opened = true;
}

bool fsFile::isOpened()
{
    return opened;
}

bool fsFile::eof()
{
    return offset < fileSize ? false : true;
}

Result fsFile::read(void *buff, u32 *readOut, u32 max)
{
    error = FSFILE_Read(fileHandle, readOut, offset, buff, max);
    if(error)
    {
        if(*readOut > max)
            *readOut = max;

        writeErrorToBuff((u8 *)buff, *readOut, error);

    }

    offset += *readOut;

    return error;
}

Result fsFile::write(void *dat, u32 *written, u32 size)
{
    *written = 0;
    error = FSFILE_Write(fileHandle, written, offset, dat, size, FS_WRITE_FLUSH);

    offset += *written;

    return error;
}

void fsFile::seek(int pos, u8 seekFrom)
{
    switch(seekFrom)
    {
        case 0:
            offset = pos;
            break;
        case 1:
            offset = offset + pos;
            break;
        case 2:
            offset = fileSize + pos;
            break;
    }
}

u8 fsFile::getByte()
{
    u8 ret;
    FSFILE_Read(fileHandle, NULL, offset, &ret, 1);
    ++offset;
    return ret;
}

void fsFile::putByte(u8 put)
{
    FSFILE_Write(fileHandle, NULL, offset, &put, 1, FS_WRITE_FLUSH);
    ++offset;
}

bool fsFile::close()
{
    error = FSFILE_Close(fileHandle);

    return error == 0;
}

u64 fsFile::size()
{
    return fileSize;
}

u64 fsFile::getOffset()
{
    return offset;
}

unsigned int fsFile::getError()
{
    return error;
}

struct
{
    bool operator()(const FS_DirectoryEntry& a, const FS_DirectoryEntry& b)
    {
        for(uint32_t i = 0; i < 0x106; i++)
        {
            int charA = tolower(a.name[i]), charB = tolower(b.name[i]);
            if(charA != charB)
                return charA < charB;
        }

        return false;
    }
} sortDirs;

dirList::dirList(FS_Archive arch, const std::u16string p)
{
    //keep archive data
    a = arch;

    //save path
    path = p;

    //open path given by p
    FSUSER_OpenDirectory(&d, a, fsMakePath(PATH_UTF16, p.data()));

    //loop until we stop reading anymore entries
    u32 read = 0;
    do
    {
        FS_DirectoryEntry getEnt;
        FSDIR_Read(d, &read, 1, &getEnt);
        if(read == 1)
            entry.push_back(getEnt);
    }
    while(read > 0);

    FSDIR_Close(d);

    std::sort(entry.begin(), entry.end(), sortDirs);
}

//clears vector used
dirList::~dirList()
{
    entry.clear();
}

//same thing as contructor. can be used to reinit dirList too
void dirList::reassign(const std::u16string p)
{
    entry.clear();

    FSUSER_OpenDirectory(&d, a, fsMakePath(PATH_UTF16, p.data()));

    u32 read = 0;
    do
    {
        FS_DirectoryEntry getEnt;
        FSDIR_Read(d, &read, 1, &getEnt);
        if(read == 1)
            entry.push_back(getEnt);
    }
    while(read > 0);

    FSDIR_Close(d);

    std::sort(entry.begin(), entry.end(), sortDirs);
}

unsigned dirList::count()
{
    return entry.size();
}

//returns true if item is a directory
bool dirList::isDir(int i)
{
    return entry[i].attributes == FS_ATTRIBUTE_DIRECTORY;
}

//returns entry's name as u16string
std::u16string dirList::retItem(int i)
{
    return std::u16string((char16_t *)entry[i].name);
}

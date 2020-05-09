#ifndef FILE_H
#define FILE_H

#include <3ds.h>
#include <string>
#include <vector>

#define seek_beg 0
#define seek_cur 1
#define seek_end 2

class fsFile
{
    public:
        //One is for reading, the other is for writing
        fsFile(FS_Archive _arch, std::u16string _path, u32 openFlags);
        fsFile(FS_Archive _arch, std::u16string _path, u32 openFlags, u64 createSize);
        //Ascii
        fsFile(FS_Archive _arch, const char *_path, u32 openFlags);
        fsFile(FS_Archive _arch, const char *_path, u32 openFlags, u64 createSize);
        //Directly
        fsFile(FS_ArchiveID _arch, FS_Path archPath, FS_Path filePath, u32 openFlags);
        //Bin
        fsFile(FS_Archive _arch, FS_Path _path, u32 openFlags);

        //returns if file is opened
        bool isOpened();
        bool eof();

        //Reads to buff. Returns number of read bytes
        Result read(void *buff, u32 *readOut, u32 max);
        //Same, only write
        Result write(void *dat, u32 *written, u32 size);
        //gets next byte in file
        u8 getByte();
        //writes byte at offset
        void putByte(u8 put);

        void seek(int pos, u8 seekFrom);

        //Closes file
        bool close();

        //returns fileSize
        u64 size();
        u64 getOffset();

        unsigned int getError();

    private:
        Handle fileHandle;
        bool opened = false;
        FS_Archive arch;
        u64 fileSize = 0, offset = 0;
        unsigned int error;
};

class dirList
{
    public:
        dirList(FS_Archive arch, const std::u16string p);
        ~dirList();
        void reassign(const std::u16string p);

        unsigned count();

        bool isDir(int i);
        std::u16string retItem(int i);

    private:
        Handle d;
        FS_Archive a;
        std::u16string path;
        std::vector<FS_DirectoryEntry> entry;
};

#endif // FILE_H

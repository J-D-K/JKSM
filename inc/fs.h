#ifndef FS_H
#define FS_H

#include <3ds.h>
#include <string>
#include "data.h"
#include "ui.h"

namespace fs
{
    void init();
    void exit();

    void fsStartSession();
    void fsEndSession();

    enum fsSeek
    {
        seek_beg,
        seek_cur,
        seek_end
    };

    FS_Archive getSDMCArch();
    FS_Archive getSaveArch();
    FS_ArchiveID getSaveMode();

    bool openArchive(data::titleData& dat, const uint32_t& arch, bool error);
    void closeSaveArch();
    void commitData(const uint32_t& mode);
    void deleteSv(const uint32_t& mode);

    inline void fcreate(const std::string& path){ FSUSER_CreateFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, path.c_str()), 0, 0); }
    inline void fdelete(const std::string& path){ FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, path.c_str())); }

    class fsfile
    {
        public:
            fsfile(const FS_Archive& _arch, const std::string& _path, const uint32_t& openFlags);
            fsfile(const FS_Archive& _arch, const std::string& _path, const uint32_t& openFlags, const uint64_t& crSize);
            fsfile(const FS_Archive& _arch, const std::u16string& _path, const uint32_t& openFlags);
            fsfile(const FS_Archive& _arch, const std::u16string& _path, const uint32_t& openFlags, const uint64_t& crSize);
            ~fsfile();

            size_t read(void *buf, const uint32_t& max);
            bool getLine(char *out, size_t max);
            size_t write(const void *buf, const uint32_t& size);
            void writef(const char *fmt, ...);

            uint8_t getByte();
            void putByte(const uint8_t& put);
            bool eof();

            void seek(const int& pos, const uint8_t& seekfrom);

            Result getError(){ return error; }
            uint64_t getOffset(){ return offset; }
            uint64_t getSize(){ return fSize; }
            bool isOpen(){ return open; }

        private:
            Handle fHandle;
            Result error;
            uint64_t fSize, offset = 0;
            bool open = false;
    };

    class dirList
    {
        public:
            dirList(const FS_Archive& arch, const std::u16string& path);
            ~dirList();

            void rescan();
            void reassign(const FS_Archive& arch, const std::u16string& p);
            const uint32_t getCount(){ return entry.size(); }
            bool isDir(unsigned i){ return entry[i].attributes == FS_ATTRIBUTE_DIRECTORY; }
            const std::u16string getItem(unsigned i){ return std::u16string((char16_t *)entry[i].name); }

        private:
            Handle d;
            FS_Archive a;
            std::u16string path;
            std::vector<FS_DirectoryEntry> entry;
    };

    void backupArchive(const std::u16string& outpath);
    void restoreToArchive(const std::u16string& inpath);

    typedef struct
    {
        FS_Archive arch;
        std::u16string from, to;
        ui::progressBar *bar;
    } backupArgs;

    backupArgs *backupArgsCreate(const FS_Archive& _arch, const std::u16string& _from, const std::u16string& _to);

    void copyFileToSD(const FS_Archive& arch, const std::u16string& from, const std::u16string& to);
    void copyFileToArch(const FS_Archive& arch, const std::u16string& from, const std::u16string& to);
    void copyDirToSD(const FS_Archive& arch, const std::u16string&  from, const std::u16string& to);
    void copyDirToArch(const FS_Archive& arch, const std::u16string& from, const std::u16string& to);

    void backupAll();
}

#endif // FS_H

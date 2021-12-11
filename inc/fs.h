#ifndef FS_H
#define FS_H

#include <3ds.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>
#include <string>
#include "data.h"
#include "ui.h"
#include "gd.h"

#define DRIVE_JKSM_DIR "JKSM"
#define DRIVE_USER_SAVE_DIR "Saves"
#define DRIVE_EXTDATA_DIR "ExtData"
#define DRIVE_SYSTEM_DIR "SysSave"
#define DRIVE_BOSS_DIR "Boss"
#define DRIVE_SHARED_DIR "Shared"

namespace fs
{
    void init();
    void exit();

    void driveInit(void *a);
    void driveExit();

    extern drive::gd *gDrive;
    extern std::string jksmDirID, usrSaveDirID, extDataDirID, sysSaveDirID, bossExtDirID, sharedExtID;

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

    bool fsfexists(const FS_Archive& _arch, const std::string& _path);
    bool fsfexists(const FS_Archive& _arch, const std::u16string& _path);
    inline void fcreate(const std::string& path){ FSUSER_CreateFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, path.c_str()), 0, 0); }
    inline void fdelete(const std::string& path){ FSUSER_DeleteFile(fs::getSDMCArch(), fsMakePath(PATH_ASCII, path.c_str())); }

    //Causes a hang for large saves, so threaded
    void delDirRec(const FS_Archive& _arch, const std::u16string& _path);
    //Threaded create dir so I can be sure it's run after ^ is finished
    void createDir(const std::string& path);
    void createDir(const FS_Archive& _arch, const std::u16string& _path);
    void createDirRec(const FS_Archive& _arch, const std::u16string& path);

    class fsfile
    {
        public:
            fsfile(const FS_Archive& _arch, const std::string& _path, const uint32_t& openFlags);
            fsfile(const FS_Archive& _arch, const std::string& _path, const uint32_t& openFlags, const uint64_t& crSize);
            fsfile(const FS_Archive& _arch, const std::u16string& _path, const uint32_t& openFlags);
            fsfile(const FS_Archive& _arch, const std::u16string& _path, const uint32_t& openFlags, const uint64_t& crSize);
            ~fsfile();
            void close();

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

    typedef struct
    {
        std::u16string name;
        bool isDir;
    } dirItem;

    class dirList
    {
        public:
            dirList() = default;
            dirList(const FS_Archive& arch, const std::u16string& path);
            ~dirList();

            void rescan();
            void reassign(const FS_Archive& arch, const std::u16string& p);
            const uint32_t getCount(){ return entry.size(); }
            bool isDir(unsigned i){ return entry[i].isDir; }
            const std::u16string getItem(unsigned i){ return entry[i].name; }
            dirItem *getDirItemAt(int i) { return &entry[i]; }

        private:
            Handle d;
            FS_Archive a;
            std::u16string path;
            std::vector<dirItem> entry;
    };

    void backupArchive(const std::u16string& outpath);
    void restoreToArchive(const std::u16string& inpath);

    void copyFile(const FS_Archive& _srcArch, const std::u16string& _src, const FS_Archive& _dstArch, const std::u16string& _dst, bool commit, threadInfo *t);
    void copyFileThreaded(const FS_Archive& _srcArch, const std::u16string& _src, const FS_Archive& _dstArch, const std::u16string& _dst, bool commit);
    void copyDirToDir(const FS_Archive& _srcArch, const std::u16string& _src, const FS_Archive& _dstArch, const std::u16string& _dst, bool commit, threadInfo *t);
    void copyDirToDirThreaded(const FS_Archive& _srcArch, const std::u16string& _src, const FS_Archive& _dstArch, const std::u16string& _dst, bool commit);

    void copyArchToZip(const FS_Archive& _arch, const std::u16string& _src, zipFile _zip, threadInfo *t);
    void copyArchToZipThreaded(const FS_Archive& _arch, const std::u16string& _src, const std::u16string& _dst);
    void copyZipToArch(const FS_Archive& _arch, unzFile _unz, threadInfo *t);
    void copyZipToArchThreaded(const FS_Archive& _arch, const std::u16string& _src);

    void backupAll();
}

#endif // FS_H

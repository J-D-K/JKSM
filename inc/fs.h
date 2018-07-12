#ifndef FS_H
#define FS_H

#include <3ds.h>
#include <string>
#include "data.h"

namespace fs
{
	void init();
	void exit();

	enum fsSeek
	{
		seek_beg,
		seek_cur,
		seek_end
	};

	FS_Archive getSDMCArch();
	FS_Archive getSaveArch();
	FS_ArchiveID getSaveMode();

	bool openArchive(data::titleData& dat, const uint32_t& arch);
	void commitData(const uint32_t& mode);
	void deleteSv(const uint32_t& mode);

	class fsfile
	{
		public:
			fsfile(const FS_Archive& _arch, const std::string& _path, const uint32_t& openFlags);
			fsfile(const FS_Archive& _arch, const std::string& _path, const uint32_t& openFlags, const uint64_t& crSize);
			fsfile(const FS_Archive& _arch, const std::u16string& _path, const uint32_t& openFlags);
			fsfile(const FS_Archive& _arch, const std::u16string& _path, const uint32_t& openFlags, const uint64_t& crSize);
			~fsfile();

			void read(uint8_t *buf, uint32_t& readOut, const uint32_t& max);
			void write(const uint8_t *buf, uint32_t& written, const uint32_t& size);
			void writeString(const std::string& str);
			uint8_t getByte();
			void putByte(const uint8_t& put);
			bool eof();

			void seek(const int& pos, const uint8_t& seekfrom);

			uint32_t getError();
			uint64_t getOffset();
			uint64_t getSize();

			bool isOpen();

		private:
			Handle fHandle;
			uint32_t error;
			uint64_t fSize, offset = 0;
			bool open = false;
	};

	class dirList
	{
		public:
			dirList(const FS_Archive& arch, const std::u16string& path);
			~dirList();

			void rescan();
			void reassign(const std::u16string& p);
			const uint32_t getCount();

			bool isDir(unsigned i);
			const std::u16string getItem(unsigned i);

		private:
			Handle d;
			FS_Archive a;
			std::u16string path;
			std::vector<FS_DirectoryEntry> entry;
	};

	void backupArchive(const std::u16string& outpath);
	void restoreToArchive(const std::u16string& inpath);

	void backupAll();
}

#endif // FS_H

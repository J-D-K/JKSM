#include <3ds.h>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include "util.h"
#include "data.h"
#include "fs.h"
#include "ui.h"
#include "gfx.h"

static const char16_t verboten[] = { L',', L'/', L'\\', L'<', L'>', L':', L'"', L'|', L'?', L'*'};

static uint32_t extdataRedirect(const uint32_t& low)
{
	//Pokemon Y
	if(low == 0x00055E00)
		return 0x0000055D;
	//Pokemon OR
	else if(low == 0x0011C400)
		return 0x000011C5;
	//Pokemon Moon
	else if(low == 0x00175E00)
		return 0x00001648;
	//Ultra Moon
	else if(low == 0x001B5100)
		return 0x00001B50;
	//Fire Emblem Conquest + SE NA
	else if(low == 0x00179600 || low == 0x00179800)
		return 0x00001794;
	//FE Conquest + SE EURO
	else if(low == 0x00179700 || low == 0x0017A800)
		return 0x00001795;
	//FE If/JPN
	else if(low == 0x0012DD00 || low == 0x0012DE00)
		return 0x000012DC;

	return (low >> 8);
}

namespace data
{
	std::vector<titleData> titles;
	std::vector<titleData> nand;
	std::vector<uint32_t> filterIds;

	titleData curData;

	bool isVerboten(const char16_t& c)
	{
		for(unsigned i = 0; i < 10; i++)
		{
			if(c == verboten[i])
				return true;
		}

		return false;
	}

	const std::u16string safeTitle(const std::u16string& s)
	{
		std::u16string ret;
		for(unsigned i = 0; i < s.length(); i++)
		{
			if(isVerboten(s[i]))
				ret += L' ';
			else
				ret += s[i];
		}
		return ret;
	}

	bool titleData::init(const uint64_t& _id, const FS_MediaType& mt)
	{
		m = mt;
		id = _id;

		low = (uint32_t)id;
		high = (uint32_t)(id >> 32);
		unique = (low >> 8);
		extdata = extdataRedirect(low);

		smdh_s *smdh = loadSMDH(low, high, m);
		if(smdh == NULL)
			return false;

		title = (char16_t *)smdh->applicationTitles[1].shortDescription;
		titleSafe.assign(safeTitle(title));
		wideTitle = util::toUtf32(title);

		char tmp[16];
		AM_GetTitleProductCode(m, id, tmp);
		prodCode = tmp;

		delete smdh;

		return true;
	}

	bool titleData::initFromCache(const uint64_t& _id, const std::u16string& _title, const std::string& code, const uint8_t& mt)
	{
		id = _id;
		low = (uint32_t)id;
		high = (uint32_t)(id >> 32);
		unique = (low >> 8);
		extdata = extdataRedirect(low);
		m = (FS_MediaType)mt;


		title.assign(_title);
		titleSafe.assign(safeTitle(title));
		wideTitle.assign(util::toUtf32(title));
		prodCode.assign(code);

		return true;
	}

	bool titleData::isOpenable()
	{
		bool ret = false;

		if(getMedia() == MEDIATYPE_GAME_CARD || getMedia() == MEDIATYPE_SD)
		{
			ret = fs::openArchive(*this, ARCHIVE_USER_SAVEDATA);
			if(!ret)
				ret = fs::openArchive(*this, ARCHIVE_EXTDATA);
		}

		if(getMedia() == MEDIATYPE_NAND)
		{
			ret = fs::openArchive(*this, ARCHIVE_SYSTEM_SAVEDATA);

			if(!ret)
				ret = fs::openArchive(*this, ARCHIVE_EXTDATA);

			if(!ret)
				ret = fs::openArchive(*this, ARCHIVE_BOSS_EXTDATA);

		}

		FSUSER_CloseArchive(fs::getSaveArch());

		return ret;
	}

	void titleData::drawInfo(unsigned x, unsigned y)
	{
		std::string media;
		switch(getMedia())
		{
			case MEDIATYPE_GAME_CARD:
				media = "Game Card";
				break;

			case MEDIATYPE_SD:
				media = "SD";
				break;

			case MEDIATYPE_NAND:
				media = "NAND";
				break;
		}

		char tmp[64];
		sprintf(tmp, "Media: %s\nHigh: 0x%08X\nLow: 0x%08X", media.c_str(), (unsigned)getHigh(), (unsigned)getLow());
		gfx::drawText(tmp, x, y, 0xFFFFFFFF);
	}

	void cartCheck()
	{
		bool ins = false;
		FSUSER_CardSlotIsInserted(&ins);

		if(titles[0].getMedia() != MEDIATYPE_GAME_CARD && ins)
		{
			uint64_t cartID = 0;
			AM_GetTitleList(NULL, MEDIATYPE_GAME_CARD, 1, &cartID);

			titleData cartData;
			if(cartData.init(cartID, MEDIATYPE_GAME_CARD))
			{
				titles.insert(titles.begin(), cartData);
				ui::loadTitleMenu();
			}
		}
		else if(titles[0].getMedia() == MEDIATYPE_GAME_CARD && !ins)
		{
			titles.erase(titles.begin(), titles.begin() + 1);
			ui::loadTitleMenu();
		}
	}

	smdh_s *loadSMDH(const uint32_t& low, const uint32_t& high, const uint8_t& media)
	{
		Handle handle;

		uint32_t archPath[] = {low, high, media, 0};
		static const uint32_t filePath[] = {0x0, 0x0, 0x2, 0x6E6F6369, 0x0};

		FS_Path binArchPath = {PATH_BINARY, 0x10, archPath};
		FS_Path binFilePath = {PATH_BINARY, 0x14, filePath};

		Result res = FSUSER_OpenFileDirectly(&handle, ARCHIVE_SAVEDATA_AND_CONTENT, binArchPath, binFilePath, FS_OPEN_READ, 0);
		if(res == 0)
		{
			uint32_t read = 0;
			smdh_s *ret = new smdh_s;
			FSFILE_Read(handle, &read, 0, ret, sizeof(smdh_s));
			FSFILE_Close(handle);

			return ret;
		}

		return NULL;
	}

	struct
	{
		bool operator()(titleData& a, titleData& b)
		{
			for(unsigned i = 0; i < a.getTitle().length(); i++)
			{
				int aChar = tolower(a.getTitle()[i]), bChar = tolower(b.getTitle()[i]);
				if(aChar != bChar)
					return aChar < bChar;
			}

			return false;
		}
	} sortTitles;

	bool checkHigh(const uint64_t& id)
	{
		uint32_t high = (uint32_t)(id >> 32);

		return (high == 0x00040000 || high == 0x00040002);
	}

	void loadTitles()
	{
		titles.clear();

		std::fstream cache(util::getBasePath() + "titles", std::ios::in | std::ios::binary);
		if(cache.is_open())
		{
			uint16_t count = 0;
			cache.read((char *)&count, sizeof(uint16_t));

			cache.seekg(1, cache.cur);

			for(unsigned i = 0; i < count; i++)
			{
				titleData newData;
				char16_t title[0x40];
				cache.read((char *)title, sizeof(char16_t) * 0x40);

				cache.seekg(1, cache.cur);

				char prodCode[16];
				cache.read(prodCode, 16);

				cache.seekg(1, cache.cur);

				uint64_t newID = 0;
				cache.read((char *)&newID, sizeof(uint64_t));

				cache.seekg(1, cache.cur);

				newData.initFromCache(newID, title, prodCode, MEDIATYPE_SD);
				titles.push_back(newData);
			}

			cache.close();
		}
		else
		{
			uint32_t count = 0;
			AM_GetTitleCount(MEDIATYPE_SD, &count);

			uint64_t *ids = new uint64_t[count];
			AM_GetTitleList(NULL, MEDIATYPE_SD, count, ids);

			ui::progressBar prog(count);

			fs::fsfile deb(fs::getSDMCArch(), "/JKSV/titledeb.txt", FS_OPEN_CREATE | FS_OPEN_WRITE);
			char tmp[256];
			sprintf(tmp, "Total titles found: %u\n", (unsigned)count);
			deb.writeString(tmp);

			for(unsigned i = 0; i < count; i++)
			{
				if(checkHigh(ids[i]))
				{
					titleData newTitle;
					sprintf(tmp, "Attempt to add: %016llX - ", ids[i]);
					deb.writeString(tmp);
					if(newTitle.init(ids[i], MEDIATYPE_SD) && newTitle.isOpenable())
					{
						titles.push_back(newTitle);
						deb.writeString("Success!\n");
					}
					else
						deb.writeString("Failed.\n");
				}

				prog.update(i);

				gfx::frameBegin();
				gfx::frameStartTop();
				ui::drawTopBar("Loading...");
				gfx::frameStartBot();
				prog.draw("Loading installed SD Titles...");
				gfx::frameEnd();
			}
			delete[] ids;

			std::sort(titles.begin(), titles.end(), sortTitles);

			cache.open(util::getBasePath() + "titles", std::ios::out | std::ios::binary);
			if(cache.is_open())
			{
				uint16_t countOut = titles.size();
				cache.write((char *)&countOut, sizeof(uint16_t));
				cache.put(0x00);

				for(unsigned i = 0; i < titles.size(); i++)
				{
					char16_t titleOut[0x40];
					std::memset(titleOut, 0, 0x40 * 2);
					std::memcpy((void *)titleOut, (void *)titles[i].getTitle().data(), titles[i].getTitle().length() * 2);
					cache.write((char *)titleOut, 0x40 * 2);
					cache.put(0x00);

					char prodOut[16];
					std::memset(prodOut, 0, 16);
					std::memcpy(prodOut, titles[i].getProdCode().c_str(), 16);
					cache.write(prodOut, 16);
					cache.put(0x00);

					uint64_t idOut = titles[i].getID();
					cache.write((char *)&idOut, sizeof(uint64_t));
					cache.put(0x00);
				}

				cache.close();
			}
		}
		curData = titles[0];
	}

	void loadNand()
	{
		nand.clear();

		std::fstream cache(util::getBasePath() + "nand", std::ios::in | std::ios::binary);
		if(cache.is_open())
		{
			uint16_t count;
			cache.read((char *)&count, sizeof(uint16_t));

			cache.seekg(1, cache.cur);

			for(uint16_t i = 0; i < count; i++)
			{
				char16_t title[0x40];
				cache.read((char *)title, 0x40 * 2);
				cache.seekg(1, cache.cur);

				char code[16];
				cache.read(code, 16);
				cache.seekg(1, cache.cur);

				uint64_t newID = 0;
				cache.read((char *)&newID, sizeof(uint64_t));
				cache.seekg(1, cache.cur);

				titleData newNand;
				newNand.initFromCache(newID, title, code, MEDIATYPE_NAND);
				nand.push_back(newNand);
			}
		}
		else
		{
			uint32_t count;
			AM_GetTitleCount(MEDIATYPE_NAND, &count);

			uint64_t *ids = new uint64_t[count];
			AM_GetTitleList(NULL, MEDIATYPE_NAND, count, ids);

			ui::progressBar prog(count);
			for(unsigned i = 0; i < count; i++)
			{
				titleData newNandTitle;
				if(newNandTitle.init(ids[i], MEDIATYPE_NAND) && newNandTitle.isOpenable() && !newNandTitle.getTitle().empty())
				{
					nand.push_back(newNandTitle);
				}
				prog.update(i);

				gfx::frameBegin();
				gfx::frameStartTop();
				ui::drawTopBar("Loading...");
				gfx::frameStartBot();
				prog.draw("Loading NAND Titles...");
				gfx::frameEnd();

			}
			delete[] ids;

			std::sort(nand.begin(), nand.end(), sortTitles);

			uint16_t countOut = nand.size();
			cache.open(util::getBasePath() + "nand", std::ios::out | std::ios::binary);
			cache.write((char *)&countOut, sizeof(uint16_t));
			cache.put(0x00);
			for(unsigned i = 0; i < nand.size(); i++)
			{
				char16_t titleOut[0x40];
				std::memset(titleOut, 0, 0x40 * 2);
				std::memcpy((void *)titleOut, nand[i].getTitle().data(), nand[i].getTitle().length() * 2);

				char prodCodeOut[16];
				std::memset(prodCodeOut, 0, 16);
				std::memcpy(prodCodeOut, nand[i].getProdCode().c_str(), nand[i].getProdCode().length());

				uint64_t idOut = nand[i].getID();

				cache.write((char *)titleOut, 0x40 * 2);
				cache.put(0x00);
				cache.write(prodCodeOut, 16);
				cache.put(0x00);
				cache.write((char *)&idOut, sizeof(uint64_t));
				cache.put(0x00);
			}
			cache.close();
		}
	}
}

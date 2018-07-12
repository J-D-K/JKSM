#ifndef DATA_H
#define DATA_H

#include <3ds.h>
#include <vector>
#include <string>

#include "smdh.h"

namespace data
{
	class titleData
	{
		public:
			bool init(const uint64_t& _id, const FS_MediaType& mt);
			bool initFromCache(const uint64_t& _id, const std::u16string& _title, const std::string& code, const uint8_t& mt);
			bool isOpenable();

			uint64_t getID();
			uint32_t getLow();
			uint32_t getHigh();
			uint32_t getUnique();
			uint32_t getExtData();
			uint8_t getMedia();

			void setExtdata(const uint32_t& ex);

			std::string getProdCode();
			std::u16string getTitle();
			std::u16string getTitleSafe();
			std::u32string getTitleWide();

			void drawInfo(unsigned x, unsigned y);

		private:
			uint64_t id;
			uint32_t high, low, unique, extdata;
			std::string prodCode;
			std::u16string title, titleSafe;
			std::u32string wideTitle;
			FS_MediaType m;
	};

	void cartCheck();

	extern std::vector<titleData> titles;
	extern std::vector<titleData> nand;
	extern titleData curData;

	smdh_s *loadSMDH(const uint32_t& low, const uint32_t& high, const uint8_t& media);
	void loadTitles();
	void loadNand();
}

#endif // DATA_H

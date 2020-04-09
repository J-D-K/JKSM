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

            uint64_t getID()
            {
                return id;
            }
            uint32_t getLow()
            {
                return low;
            }
            uint32_t getHigh()
            {
                return high;
            }
            uint32_t getUnique()
            {
                return unique;
            }
            uint32_t getExtData()
            {
                return extdata;
            }
            uint8_t getMedia()
            {
                return m;
            }
            bool getFav()
            {
                return fav;
            }
            void setFav(bool _set)
            {
                fav = _set;
            }

            void setExtdata(const uint32_t& ex)
            {
                extdata = ex;
            }

            std::string getProdCode()
            {
                return prodCode;
            }
            std::u16string getTitle()
            {
                return title;
            }
            std::u16string getTitleSafe()
            {
                return titleSafe;
            }

            void drawInfo(unsigned x, unsigned y);

        private:
            uint64_t id;
            uint32_t high, low, unique, extdata;
            std::string prodCode;
            std::u16string title, titleSafe;
            FS_MediaType m;
            bool fav = false;
    };

    void cartCheck();

    extern std::vector<titleData> titles;
    extern std::vector<titleData> nand;
    extern titleData curData;

    smdh_s *loadSMDH(const uint32_t& low, const uint32_t& high, const uint8_t& media);
    void loadTitles();
    void loadNand();

    void loadBlacklist();
    void blacklistAdd(titleData& t);

    void loadFav();
    void saveFav();
    void favAdd(titleData& t);
    void favRem(titleData& t);

    //Writes title data cache to path
    bool readCache(std::vector<titleData>& t, const std::string& path, bool nand);
    void createCache(std::vector<titleData>& t, const std::string& path);

    extern uint8_t lang;
}

#endif // DATA_H

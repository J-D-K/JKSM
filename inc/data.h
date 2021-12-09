#ifndef DATA_H
#define DATA_H

#include <3ds.h>
#include <citro2d.h>
#include <vector>
#include <string>

#include "smdh.h"

namespace data
{
    void init();
    void exit();

    typedef struct
    {
        bool hasUser = false;
        bool hasExt  = false;
        bool hasSys  = false;
        bool hasBoss = false;
    } titleSaveTypes;

    class titleData
    {
        public:
            bool init(const uint64_t& _id, const FS_MediaType& mt);
            bool initFromCache(const uint64_t& _id, const std::u16string& _title, const std::u16string& _pub, const std::string& code, const data::titleSaveTypes& _st, const uint8_t& mt);
            void testMounts();
            bool hasSaveData();

            uint64_t getID() const { return id; }
            uint32_t getLow() const { return low; }
            uint32_t getHigh() const { return high; }
            uint32_t getUnique() const { return unique; }
            uint32_t getExtData() const { return extdata; }
            uint8_t getMedia() const { return m; }
            bool getFav() const { return fav; }

            void setFav(bool _set) { fav = _set; }
            void setExtdata(const uint32_t& ex) { extdata = ex; }
            void setTitle(const std::u16string& _t);

            std::string getProdCode() { return prodCode; }
            std::string getIDStr()    { return idStr; }
            std::u16string getTitle() { return title; }
            std::u16string getTitleSafe() { return titleSafe; }
            std::u16string getPublisher() { return publisher; }
            data::titleSaveTypes getSaveTypes() { return types; }

            void drawInfo(unsigned x, unsigned y);
            void setIcon(C2D_Image _set) { icon = _set; }
            uint8_t *getIconData() { return (uint8_t *)icon.tex->data; }
            C2D_Image *getIcon() { return &icon; }
            void drawIconAt(float x, float y, uint16_t w, uint16_t h, float depth);

            void assignIcon(C3D_Tex *_icon);
            void freeIcon() { if(icon.tex) { C3D_TexDelete(icon.tex); } }

        private:
            uint64_t id;
            uint32_t high, low, unique, extdata;
            std::string prodCode, idStr;
            std::u16string title, titleSafe, publisher;
            FS_MediaType m;
            bool fav = false;
            titleSaveTypes types;
            C2D_Image icon = {NULL, NULL};
    };

    void cartCheck();

    extern std::vector<titleData> usrSaveTitles;
    extern std::vector<titleData> extDataTitles;
    extern std::vector<titleData> sysDataTitles;
    extern std::vector<titleData> bossDataTitles;
    extern titleData curData;

    smdh_s *loadSMDH(const uint32_t& low, const uint32_t& high, const uint8_t& media);
    void loadTitles(void *a);

    void loadBlacklist();
    void saveBlacklist();
    void blacklistAdd(titleData& t);

    void loadFav();
    void saveFav();
    void favAdd(titleData& t);
    void favRem(titleData& t);

    //Reads icon to C2D_image
    C2D_Image readIconFromSMDH(smdh_s *smdh);

    //Writes title data cache to path
    bool readCache(std::vector<titleData>& vect, const std::string& path, bool nand);
    void createCache(std::vector<titleData>& vect, const std::string& path);

    //Just functions to draw while data load thread runs
    void datDrawTop();
    void datDrawBot();
}

#endif // DATA_H

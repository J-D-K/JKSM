#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

#include "type.h"

namespace ui
{
    typedef struct
    {
        funcPtr func = NULL;
        void *args   = NULL;
        uint32_t button = 0;
    } menuOptEvent;

    typedef struct
    {
        std::string txt;
        std::vector<menuOptEvent> events;
    } menuOpt;

    class menu
    {
        public:
            int addOpt(const std::string& add, int maxWidth);
            void addOptEvent(unsigned ind, uint32_t _key, funcPtr _func, void *_args);
            void editOpt(int ind, const std::string& ch){ opt[ind].txt = ch; }
            void reset();
            void adjust();
            void setSelected(const int& newSel);

            void update();
            void draw(const int& x, const int&y, const uint32_t& baseClr, const uint32_t& rectWidth, bool lightBack);

            int getSelected() { return selected; }
            unsigned getCount() { return opt.size(); }
            std::string getOpt(const int& g) { return opt[g].txt; }

            void setCallback(funcPtr _cb, void *_args) { cb = _cb; args = _args; }

        private:
            uint8_t clrSh = 0x88;
            bool clrAdd = true, multi = false;
            int selected = 0, start = 0;
            int fc = 0;
            funcPtr cb;
            void *args;
            std::vector<menuOpt> opt;
    };
}

#endif // MENU_H

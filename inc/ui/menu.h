#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

namespace ui
{
    class menu
    {
        public:
            void addOpt(const std::string& add, int maxWidth);
            void editOpt(int ind, const std::string& ch)
            {
                opt[ind] = ch;
            }
            void multiSet(const bool& s);
            bool multiIsSet(int ind)
            {
                return multiSel[ind];
            }
            void reset();
            void adjust();
            void setSelected(const int& newSel);

            void handleInput(const uint32_t& down, const uint32_t& held);
            void draw(const int& x, const int&y, const uint32_t& baseClr, const uint32_t& rectWidth, bool lightBack);

            int getSelected()
            {
                return selected;
            }
            unsigned getCount()
            {
                return opt.size();
            }
            std::string getOpt(const int& g)
            {
                return opt[g];
            }

        private:
            uint8_t clrSh = 0x88;
            bool clrAdd = true, multi = false;
            int selected = 0, start = 0;
            int fc = 0;
            std::vector<std::string> opt;
            std::vector<bool> multiSel;
    };
}

#endif // MENU_H

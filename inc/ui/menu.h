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
            void reset();
            void setSelected(const int& newSel);

            void handleInput(const uint32_t& key, const uint32_t& held);
            void draw(const int& x, const int&y, const uint32_t& baseClr, const uint32_t& rectWidth);

            int getSelected() { return selected; }
            unsigned getCount() { return opt.size(); }
            std::string getOpt(const int& g) { return opt[g]; }

        private:
            uint8_t clrSh = 0;
            bool clrAdd = true;
            int selected = 0, start = 0;
            int fc = 0;
            std::vector<std::string> opt;
    };
}

#endif // MENU_H

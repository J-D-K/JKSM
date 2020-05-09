#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

class menuItem
{
    public:
        menuItem(const std::u32string s, bool center, int x);
        void autoCenter();
        void draw(int y, unsigned color);

        bool selected;
        std::u32string text;

        int x;
};

class menu
{
    public:
        menu(unsigned sx, unsigned sy, bool sMulti, bool _center);
        ~menu();

        void addItem(const char *a);
        void addItem(const std::u16string a);
        void addItem(const std::u32string a);

        void updateItem(int i, const char *a);

        int getSelected();
        void setSelected(int sel);
        void reset();

        void centerOpts();
        void autoVert();

        unsigned getSize();
        unsigned getSelectCount();
        bool optSelected(int i);

        void draw();

        void handleInput(u32 key, u32 held);

    private:
        bool multi, center;
        std::vector<menuItem> opts;
        int x, y, selected, start, fc;
};

void prepMain(), prepBackMenu(), prepSaveMenu();
void prepExtMenu(), prepNandBackup(), prepSharedMenu();
void prepSharedBackMenu(), prepExtras(), prepDevMenu();

#endif // MENU_H

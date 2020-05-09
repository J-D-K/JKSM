#ifndef UI_H
#define UI_H

#include <string>
#include <vector>

#include "data.h"
#include "ui/button.h"
#include "ui/menu.h"

enum states
{
    MAIN_MENU,
    TITLE_MENU,
    BACK_MENU,
    SYS_MENU,
    SYS_BAKMENU,
    FLDR_MENU,
    ADV_MENU
};

namespace ui
{
    void prepMenus();
    void loadTitleMenu();
    void showMessage(const char *fmt, ...);
    void drawTopBar(const std::string& info);
    void runApp(const uint32_t& down, const uint32_t& held);

    std::u16string getFolder(const data::titleData& dat, const uint32_t& mode, const FS_Archive& arch, const bool& newFolder);

    void advModePrep();
    void stateAdvMode(const uint64_t& down, const uint64_t& held);

    class progressBar
    {
        public:
            progressBar(const uint32_t& _max);
            void update(const uint32_t& _prog);
            void draw(const std::string& text);

        private:
            float max, prog, width;
    };

    bool confirm(const std::string& mess);
}

#endif // UI_H

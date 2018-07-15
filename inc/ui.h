#ifndef UI_H
#define UI_H

#include <string>
#include <vector>

#include "data.h"
#include "ui/button.h"
#include "ui/menu.h"

namespace ui
{
    void prepMenus();
    void loadTitleMenu();
    void showMessage(const std::string& mess);
    void drawTopBar(const std::string& info);
    void runApp(const uint32_t& down, const uint32_t& held);

    //These are locked into because the archive is opened. NOT ANYMORE THANK YOU SWITCH VERSION
    std::u16string getFolder(const data::titleData& dat, const uint32_t& mode, const FS_Archive& arch, const bool& newFolder);
    void advMode(const FS_Archive& arch);

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

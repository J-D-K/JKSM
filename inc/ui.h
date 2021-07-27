#ifndef UI_H
#define UI_H

#include <string>
#include <vector>

#include "data.h"
#include "type.h"

#include "ui/button.h"
#include "ui/menu.h"
#include "ui/ttlview.h"
#include "ui/ttl.h"
#include "ui/ext.h"
#include "ui/sysview.h"
#include "ui/boss.h"
#include "ui/shrd.h"
#include "ui/fld.h"

enum states
{
    DAT,
    USR,
    EXT,
    SYS,
    BOS,
    SHR
};

extern const std::string TITLE_TEXT;

namespace ui
{
    enum
    {
        SCREEN_TOP,
        SCREEN_BOT
    };

    extern uint32_t down, held;
    inline void updateInput()
    {
        hidScanInput();
        down = hidKeysDown();
        held = hidKeysHeld();
    }
    inline uint32_t padKeysDown(){ return down; }
    inline uint32_t padKeysHeld(){ return held; }

    void init();
    void exit();
    void showMessage(const char *fmt, ...);
    void drawTopBar(const std::string& info);
    void newThread(ThreadFunc _thrdFunc, void *_args, funcPtr _drawFunc);
    bool runApp();

    std::u16string getFolder(const data::titleData& dat, const uint32_t& mode, const FS_Archive& arch, const bool& newFolder);

    void advModePrep();
    void stateAdvMode(const uint64_t& down, const uint64_t& held);
    void drawUIBar(const std::string& txt, int screen, bool center);

    class progressBar
    {
        public:
            progressBar() = default;

            progressBar(const uint32_t& _max);
            void setMax(const uint32_t& _max) { max = _max; }
            void update(const uint32_t& _prog);
            void draw();

        private:
            float max, prog, width;
    };

    bool confirm(const std::string& mess);
    extern const std::string loadGlyphArray[];
    extern int state, prev;
}

#endif // UI_H

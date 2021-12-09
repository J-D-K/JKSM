#ifndef UI_H
#define UI_H

#include <string>
#include <vector>

#include "data.h"
#include "thrdMgr.h"
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
#include "ui/set.h"

enum states
{
    DAT,
    USR,
    EXT,
    SYS,
    BOS,
    SHR,
    SET
};

extern const std::string TITLE_TEXT;

#define FLD_GUIDE_TEXT "\ue000 Select \ue002 Delete \ue003 Restore \ue005 Upload \ue001 Close"

namespace ui
{
    enum
    {
        SCREEN_TOP,
        SCREEN_BOT
    };

    typedef struct 
    {
        std::string q;
        funcPtr onConfirm = NULL, onCancel = NULL;
        void *args = NULL;
    } confArgs;
    
    extern uint32_t down, held;
    extern touchPosition p;
    inline void updateInput()
    {
        hidScanInput();
        down = hidKeysDown();
        held = hidKeysHeld();
        touchRead(&p);
    }

    inline uint32_t padKeysDown(){ return down; }
    inline uint32_t padKeysHeld(){ return held; }
    inline touchPosition touchPosition() { return p; }

    void init();
    void exit();
    void showMessage(const char *fmt, ...);
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
            void draw(const std::string& text);

        private:
            float max, prog, width;
    };

    void confirm(const std::string& mess, funcPtr _onConfirm, funcPtr _onCancel, void *args);
    extern const std::string loadGlyphArray[];
    extern int state, prev;
}

#endif // UI_H

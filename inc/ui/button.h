#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include <3ds.h>

//Reuse update from Switch version
enum buttonEvents
{
    BUTTON_NOTHING,
    BUTTON_PRESSED,
    BUTTON_RELEASED
};

namespace ui
{
    class button
    {
        public:
            button(const std::string& _txt, unsigned _x, unsigned _y, unsigned _w, unsigned _h);
            void update(const touchPosition& p);
            bool isOver();
            bool wasOver();
            int getEvent()
            {
                return retEvent;
            }

            void draw();

            unsigned getX()
            {
                return x;
            }
            unsigned getY()
            {
                return y;
            }
            unsigned getTx()
            {
                return tx;
            }
            unsigned getTy()
            {
                return ty;
            }

        protected:
            bool pressed = false, first = false;
            int retEvent = BUTTON_NOTHING;
            unsigned x, y, w, h;
            unsigned tx, ty;
            std::string text;
            touchPosition prev, cur;
    };
}

#endif // BUTTON_H

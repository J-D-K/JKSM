#ifndef UI_H
#define UI_H

#include <3ds.h>
#include <string>

bool confirm(const char *t);
void showMessage(const char *t, const char *head);
void showError(const char *t, unsigned error);

class textbox
{
    public:
        textbox(unsigned x, unsigned y, unsigned width, unsigned height, const char *text, const char *_head);
        void changeText(const char *nText);
        void draw();

        unsigned X, Y;
    protected:
        unsigned Width, Height;
        unsigned headX;
        float xScale = 0, yScale = 0;
        std::string Text, head;
};

class progressBar
{
    public:
        progressBar(float _max, const char *t, const char *head);
        ~progressBar();
        void draw(float cur);

    private:
        textbox *back;
        float max;
};

class button
{
    public:
        button(const char *sText, int sX, int sY, int sWidth, int sHeight);
        void draw();
        bool isOver(touchPosition p);
        bool released(touchPosition p);

    private:
        bool Pressed;
        int X, Y, width, height;
        int textX, textY;
        std::string text;
        touchPosition Prev;
};

void textboxInit();
void textboxExit();
void progressBarInit();
void progressBarExit();
void topBarInit();
void topBarExit();
void drawTopBar(const std::u32string nfo);

#endif // UI_H

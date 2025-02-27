#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include <M5Unified.h>
#include "Button.h"
#include "Constants.h"

#define ACTIONBAR_BACK_BUTTON_WIDTH 60
#define ACTIONBAR_TEXT_SIZE 2

class ActionBar {
public:
    ActionBar();
    void begin();
    void setTitle(String title);
    void setStatus(String status);
    void draw();
    bool isBackPressed();

private:
    String title;
    String status;
    Button backButton = Button(0, 0, ACTIONBAR_BACK_BUTTON_WIDTH, ACTIONBAR_HEIGHT, "<");
    int backButtonWidth;
    int barHeight;
    int textSize;
    bool backPressed;
};

#endif // ACTIONBAR_H

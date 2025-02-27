#ifndef BUTTON_H
#define BUTTON_H

#include <M5Unified.h>

class Button {
public:
    Button(int x, int y, int w, int h, const char* label);
    void begin();
    void draw();
    bool isPressed();
    void setLabel(const char* newLabel);
    void setColor(uint16_t normal, uint16_t pressed);

private:
    int posX, posY, width, height;
    String label;
    uint16_t normalColor, pressedColor;
    unsigned long lastPressTime;
    bool wasPressed;
};

#endif // BUTTON_H

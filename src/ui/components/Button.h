#ifndef BUTTON_H
#define BUTTON_H

#include <M5Unified.h>

#define BUTTON_TYPE_OUTLINE 0
#define BUTTON_TYPE_TEXT 1

class Button {
public:
    Button(int x, int y, int w, int h, const char* label);
    void begin();
    void draw();
    bool isPressed();
    void setLabel(const char* newLabel);
    void setColor(uint16_t normal, uint16_t pressed);
    void setFontSize(uint8_t size);
    void setType(uint8_t type) { this->type = type; }
    void setId(int id) { this->id = id; }
    int getId() const { return id; }
    bool containsPoint(int x, int y) const;
    void setPressed(bool pressed);
    
private:
    int id;
    int posX, posY, width, height;
    String label;
    uint16_t normalColor, pressedColor;
    unsigned long lastPressTime;
    bool wasPressed;
    uint8_t fontSize;
    uint8_t type;
};

#endif // BUTTON_H

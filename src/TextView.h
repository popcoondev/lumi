#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <M5Unified.h>

class TextView {
public:
    TextView();
    void begin();
    void setText(const char* text);
    void setPosition(int x, int y, int width, int height);
    void setFontSize(int size);
    void setColor(uint16_t color);
    void setBackgroundColor(uint16_t color);
    void clear();
    void draw();

private:
    int posX, posY, width, height;
    int fontSize;
    String currentText;
    bool needsUpdate;
    std::vector<String> wrappedText; // 改行済みテキスト
    uint16_t color;
    uint16_t backgroundColor;

    void wrapText();
};

#endif // TEXTVIEW_H

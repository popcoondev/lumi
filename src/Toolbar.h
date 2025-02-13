#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <Arduino.h>
#include <M5Unified.h>
#include "Button.h"

// 画面サイズ
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// ツールバーの設定
#define TOOLBAR_HEIGHT 60
#define BUTTON_COUNT 3
#define BUTTON_WIDTH (SCREEN_WIDTH / BUTTON_COUNT)

// ボタンのID
enum ButtonID { BTN_A, BTN_B, BTN_C, BTN_NONE };

// ツールバークラス
class Toolbar {
private:
    Button buttons[BUTTON_COUNT] = {
        Button(0, SCREEN_HEIGHT - TOOLBAR_HEIGHT, BUTTON_WIDTH, TOOLBAR_HEIGHT, "A"),
        Button(BUTTON_WIDTH, SCREEN_HEIGHT - TOOLBAR_HEIGHT, BUTTON_WIDTH, TOOLBAR_HEIGHT, "B"),
        Button(BUTTON_WIDTH * 2, SCREEN_HEIGHT - TOOLBAR_HEIGHT, BUTTON_WIDTH, TOOLBAR_HEIGHT, "C")
    };
    
public:
    Toolbar();  
    void begin();  
    void setButtonLabel(ButtonID button, String label);  
    bool getPressedButton(ButtonID button);
    void draw();  
};

#endif // TOOLBAR_H

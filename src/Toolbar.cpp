#include "Toolbar.h"

Toolbar::Toolbar() {}

void Toolbar::begin() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].begin();
    }
    draw();
}

// ボタンのラベルを変更
void Toolbar::setButtonLabel(ButtonID button, String label) {
    if (button >= 0 && button < BUTTON_COUNT) {
        buttons[button].setLabel(label.c_str());
        draw();
    }
}

// タッチ検知 & ボタンイベント取得
ButtonID Toolbar::getPressedButton() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (buttons[i].isPressed()) {
            return static_cast<ButtonID>(i);
        }
    }
    return BTN_NONE;
}

// ツールバーの描画
void Toolbar::draw() {
    M5.Lcd.fillRect(0, SCREEN_HEIGHT - TOOLBAR_HEIGHT, SCREEN_WIDTH, TOOLBAR_HEIGHT, DARKGREY);
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].draw();
    }
}

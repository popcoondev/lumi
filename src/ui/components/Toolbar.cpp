#include "Toolbar.h"

Toolbar::Toolbar() {}

void Toolbar::begin() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].begin();
    }
}

// ボタンのラベルを変更
void Toolbar::setButtonLabel(ButtonID button, String label) {
    if (button >= 0 && button < BUTTON_COUNT) {
        buttons[button].setLabel(label.c_str());
    }
}

// タッチ検知 & ボタンイベント取得
bool Toolbar::getPressedButton(ButtonID button) {
    if (button >= 0 && button < BUTTON_COUNT) {
        if (buttons[button].isPressed()) {
            return true;
        }
    }
    return false;
}

// ツールバーの描画
void Toolbar::draw() {
    M5.Lcd.fillRect(0, SCREEN_HEIGHT - TOOLBAR_HEIGHT, SCREEN_WIDTH, TOOLBAR_HEIGHT, DARKGREY);
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].draw();
    }
}

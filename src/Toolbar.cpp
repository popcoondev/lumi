#include "Toolbar.h"

Toolbar::Toolbar() {
    buttonLabels[0] = "A";
    buttonLabels[1] = "B";
    buttonLabels[2] = "C";
    lastPressed = BTN_NONE;
}

// 初期化
void Toolbar::begin() {
    draw();
}

// ボタンのラベルを設定
void Toolbar::setButtonLabel(ButtonID button, String label) {
    if (button >= 0 && button < BUTTON_COUNT) {
        buttonLabels[button] = label;
        draw();
    }
}

// タッチ検知 & ボタンイベント取得
ButtonID Toolbar::getPressedButton() {
    M5.update(); // **タッチイベントを更新**

    if (M5.Touch.getCount() > 0) {
        auto touch = M5.Touch.getDetail();
        int x = touch.x;
        int y = touch.y;

        Serial.printf("Touch Detected at (%d, %d)\n", x, y); // タップ位置をデバッグ出力

        // **ツールバーの範囲外なら無視**
        if (y < SCREEN_HEIGHT - TOOLBAR_HEIGHT) {
            return BTN_NONE;
        }

        // **ボタンの範囲内で判定**
        if (x >= 0 && x < BUTTON_WIDTH) {
            Serial.println("Button A Pressed");
            lastPressed = BTN_A;
        } else if (x >= BUTTON_WIDTH && x < BUTTON_WIDTH * 2) {
            Serial.println("Button B Pressed");
            lastPressed = BTN_B;
        } else if (x >= BUTTON_WIDTH * 2 && x < SCREEN_WIDTH) {
            Serial.println("Button C Pressed");
            lastPressed = BTN_C;
        } else {
            return BTN_NONE;  // **範囲外なら無視**
        }
        return lastPressed;
    }
    return BTN_NONE;
}

// ツールバーの描画
void Toolbar::draw() {
    M5.Lcd.fillRect(0, SCREEN_HEIGHT - TOOLBAR_HEIGHT, SCREEN_WIDTH, TOOLBAR_HEIGHT, DARKGREY);
    for (int i = 0; i < BUTTON_COUNT; i++) {
        int x = i * BUTTON_WIDTH;
        M5.Lcd.fillRect(x, SCREEN_HEIGHT - TOOLBAR_HEIGHT, BUTTON_WIDTH - 2, TOOLBAR_HEIGHT - 2, LIGHTGREY);
        M5.Lcd.setCursor(x + 10, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 15);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(BLACK);
        M5.Lcd.print(buttonLabels[i]);
    }
}

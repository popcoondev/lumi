#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <Arduino.h>
#include <M5Unified.h>

// 画面サイズ
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// ツールバーの設定
#define TOOLBAR_HEIGHT 50
#define BUTTON_COUNT 3
#define BUTTON_WIDTH (SCREEN_WIDTH / BUTTON_COUNT)

// ボタンのID
enum ButtonID { BTN_A, BTN_B, BTN_C, BTN_NONE };

// ツールバークラス
class Toolbar {
private:
    String buttonLabels[BUTTON_COUNT];  // ボタンのラベル
    ButtonID lastPressed;

public:
    Toolbar();  // コンストラクタ
    void begin();  // 初期化
    void setButtonLabel(ButtonID button, String label);  // ボタンラベル設定
    ButtonID getPressedButton();  // ボタンの検知
    void draw();  // UIの描画
};

#endif

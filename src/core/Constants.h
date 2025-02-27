#ifndef CONSTANTS_H
#define CONSTANTS_H

// ハードウェア定数
#define LED_PIN 8           // DIN Base Port.B
#define MAX_FACES 8         // 最大面数
#define LED_ADDRESS_OFFSET 1  // LEDアドレスのオフセット（0番は未使用）
#define NUM_LEDS (8 * 2) + LED_ADDRESS_OFFSET  // LEDテープ全体のLED数

// センサーしきい値
#define STABLE_THRESHOLD 0.2 
#define STABLE_DURATION 4000  // 4秒間
#define TIMEOUT_DURATION 120000  // 2分間

// UI定数
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
// ActionBar.hの定義に合わせる
#define ACTIONBAR_HEIGHT 40
// Toolbar.hの定義に合わせる
#define TOOLBAR_HEIGHT 60

#endif // CONSTANTS_H
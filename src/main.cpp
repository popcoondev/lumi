#if 1
#include <Arduino.h>
#include <M5Unified.h>
#include "OctaController.h"

// メインコントローラのインスタンス
OctaController* controller = nullptr;

void setup() {
  // M5.begin()はOctaController内で呼び出されるため、ここでは呼ばない
  
  // コントローラの作成
  controller = new OctaController();
  
  // コントローラの初期化
  controller->setup();
}

void loop() {
  // M5.update()はUIManagerクラス内で呼び出される
  
  // コントローラのメインループを実行
  controller->loop();
}
#endif
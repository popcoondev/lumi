#if 0
#include <Arduino.h>
#include <M5Unified.h>
#include "led/LEDManager.h"
#include "led/JsonLEDPatternAdapter.h"

// LEDマネージャーのインスタンス
LEDManager ledManager;

// JSONパターンアダプターのインスタンス
JsonLedPatternAdapter* jsonPattern = nullptr;

void setup() {
    // M5Stackの初期化
    auto cfg = M5.config();
    M5.begin(cfg);
    
    // シリアル通信の初期化
    Serial.begin(115200);
    Serial.println("JSON LED Pattern Adapter Test");
    
    // LEDマネージャーの初期化
    ledManager.begin(LED_PIN, NUM_LEDS, LED_ADDRESS_OFFSET);
    
    // JSONパターンアダプターの作成
    jsonPattern = new JsonLedPatternAdapter("JSON Test Pattern");
    
    // パターンの実行
    M5.Lcd.println("Running JSON Pattern...");
    M5.Lcd.println("Press A to stop, B to restart");
    
    // パターンを実行するためのタスクを作成
    TaskHandle_t taskHandle = nullptr;
    xTaskCreatePinnedToCore(
        [](void* parameter) {
            JsonLedPatternAdapter* pattern = static_cast<JsonLedPatternAdapter*>(parameter);
            pattern->run(
                ledManager.getLeds(),
                ledManager.getNumLeds(),
                ledManager.getLedOffset(),
                ledManager.getNumFaces(),
                0  // 無限実行
            );
            vTaskDelete(NULL);
        },
        "JSONPatternTask",
        4096,
        jsonPattern,
        1,
        &taskHandle,
        1
    );
}

void loop() {
    // ボタン入力の更新
    M5.update();
    
    // Aボタンでパターン停止
    if (M5.BtnA.wasPressed()) {
        Serial.println("Stopping pattern");
        M5.Lcd.println("Pattern stopped");
        ledManager.stopPattern();
    }
    
    // Bボタンでパターン再開
    if (M5.BtnB.wasPressed()) {
        Serial.println("Starting pattern");
        M5.Lcd.println("Pattern started");
        
        // パターンを実行するためのタスクを作成
        TaskHandle_t taskHandle = nullptr;
        xTaskCreatePinnedToCore(
            [](void* parameter) {
                JsonLedPatternAdapter* pattern = static_cast<JsonLedPatternAdapter*>(parameter);
                pattern->run(
                    ledManager.getLeds(),
                    ledManager.getNumLeds(),
                    ledManager.getLedOffset(),
                    ledManager.getNumFaces(),
                    0  // 無限実行
                );
                vTaskDelete(NULL);
            },
            "JSONPatternTask",
            4096,
            jsonPattern,
            1,
            &taskHandle,
            1
        );
    }
    
    // 少し待機
    delay(10);
}
#endif
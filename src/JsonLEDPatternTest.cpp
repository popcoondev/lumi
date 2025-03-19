#if 0
#include <Arduino.h>
#include <M5Unified.h>
#include <SPIFFS.h>
#include "led/LEDManager.h"
#include "core/Constants.h"

// LEDマネージャーのインスタンス
LEDManager ledManager;

// SPIFFSからJSONファイルを読み込む関数
String loadJsonFromSPIFFS(const char* path) {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return "";
    }
    
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return "";
    }
    
    String json = file.readString();
    file.close();
    return json;
}

void setup() {
    // M5Stackの初期化
    auto cfg = M5.config();
    M5.begin(cfg);
    
    // シリアル通信の初期化
    Serial.begin(115200);
    Serial.println("JSON LED Pattern Test");
    
    // LEDマネージャーの初期化
    ledManager.begin(LED_PIN, NUM_LEDS, LED_ADDRESS_OFFSET);
    
    // JSONパターンの読み込み
    String jsonPattern = loadJsonFromSPIFFS("/led_patterns.json");
    if (jsonPattern.length() > 0) {
        Serial.println("JSON Pattern loaded:");
        Serial.println(jsonPattern);
        
        // JSONパターンの解析
        bool success = ledManager.loadJsonPatternsFromString(jsonPattern);
        if (success) {
            Serial.println("JSON Pattern parsed successfully");
            Serial.print("Pattern count: ");
            Serial.println(ledManager.getJsonPatternCount());
            
            // パターン名の表示
            for (int i = 0; i < ledManager.getJsonPatternCount(); i++) {
                Serial.print("Pattern ");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(ledManager.getJsonPatternName(i));
            }
            
            // パターンの実行
            Serial.println("Running JSON Pattern...");
            ledManager.runJsonPatternByIndex(0);
        } else {
            Serial.println("Failed to parse JSON Pattern");
        }
    } else {
        Serial.println("Failed to load JSON Pattern");
    }
}

void loop() {
    // ボタン入力の更新
    M5.update();
    
    // Aボタンでパターン停止
    if (M5.BtnA.wasPressed()) {
        Serial.println("Stopping pattern");
        ledManager.stopPattern();
    }
    
    // Bボタンでパターン再開
    if (M5.BtnB.wasPressed()) {
        Serial.println("Starting pattern");
        ledManager.runJsonPatternByIndex(0);
    }
    
    // 少し待機
    delay(10);
}
#endif
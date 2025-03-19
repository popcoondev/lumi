#ifndef JSON_LED_PATTERN_ADAPTER_H
#define JSON_LED_PATTERN_ADAPTER_H

#include <Arduino.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include "LEDManager.h"

// LedPatternを継承するJSONパターンアダプタークラス
class JsonLedPatternAdapter : public LedPattern {
public:
    JsonLedPatternAdapter(const String& name) : m_name(name) {}
    
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override {
        // JSONパターンの実行ロジックをここに実装
        // 例: ランダムな色で点滅するシンプルなパターン
        unsigned long startTime = millis();
        
        while (true) {
            // 実行時間チェック
            if (duration > 0 && millis() - startTime >= (unsigned long)duration) {
                break;
            }
            
            // ランダムな色を生成
            CHSV color(random(0, 255), 255, 255);
            
            // すべての面を同じ色に設定
            for (int i = 0; i < numFaces; i++) {
                int idx1 = ledOffset + (i * 2);
                int idx2 = ledOffset + (i * 2) + 1;
                
                leds[idx1] = color;
                leds[idx2] = color;
            }
            
            // LEDの表示
            FastLED.show();
            
            // 少し待機
            vTaskDelay(500 / portTICK_PERIOD_MS);
            
            // すべての面を消灯
            for (int i = 0; i < numFaces; i++) {
                int idx1 = ledOffset + (i * 2);
                int idx2 = ledOffset + (i * 2) + 1;
                
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
            
            // LEDの表示
            FastLED.show();
            
            // 少し待機
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
    
    String getName() override {
        return m_name;
    }
    
private:
    String m_name;
};

#endif // JSON_LED_PATTERN_ADAPTER_H

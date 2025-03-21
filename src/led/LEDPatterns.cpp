#include "LEDManager.h"

// シーケンシャルパターンの実装
void SequentialPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < numFaces; j++) {
            int idx1 = ledOffset + (j * 2);
            int idx2 = ledOffset + (j * 2) + 1;
            if (j <= i) {
                leds[idx1] = CRGB::White;
                leds[idx2] = CRGB::White;
            } else {
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
        }
        FastLED.show();
        for (int k = 0; k < 10; k++) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

// On/Offパターンの実装
void OnOffPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    bool state = true;
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            if (state) {
                leds[idx1] = CRGB::White;
                leds[idx2] = CRGB::White;
            } else {
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
        }
        FastLED.show();
        state = !state;
        for (int k = 0; k < 10; k++) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

// 奇数/偶数パターンの実装
void OddEvenPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    bool toggle = false;
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        toggle = !toggle;
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            if (toggle) {
                if (i % 2 == 0) {
                    leds[idx1] = CRGB::White;
                    leds[idx2] = CRGB::White;
                } else {
                    leds[idx1] = CRGB::Black;
                    leds[idx2] = CRGB::Black;
                }
            } else {
                if (i % 2 == 1) {
                    leds[idx1] = CRGB::White;
                    leds[idx2] = CRGB::White;
                } else {
                    leds[idx1] = CRGB::Black;
                    leds[idx2] = CRGB::Black;
                }
            }
        }
        FastLED.show();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// ランダムパターンの実装
void RandomPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            CRGB randColor = CRGB(random(256), random(256), random(256));
            leds[idx1] = randColor;
            leds[idx2] = randColor;
        }
        FastLED.show();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

// ウェーブパターンの実装
void WavePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    uint8_t wavePos = 0;
    
    // 明るさのテーブルを直接定義（デバッグ用に明確な値を使用）
    uint8_t brightnessTable[8] = {255, 220, 180, 140, 100, 140, 180, 220};
    
    Serial.println("WavePattern: Starting pattern");
    Serial.printf("WavePattern: numLeds=%d, ledOffset=%d, numFaces=%d\n", numLeds, ledOffset, numFaces);
    
    // 最初に全LEDを消灯して状態を確認
    for (int i = 0; i < numLeds; i++) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
    Serial.println("WavePattern: All LEDs set to black");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    // テスト用に全LEDを白色に点灯
    for (int i = 0; i < numLeds; i++) {
        leds[i] = CRGB::White;
    }
    FastLED.show();
    Serial.println("WavePattern: All LEDs set to white for testing");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    while (millis() - startTime < duration) {
        // デバッグ出力
        Serial.println("WavePattern: Updating LEDs");
        
        // 各フェイスの明るさを更新
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            
            // インデックスの範囲チェック
            if (idx1 >= numLeds || idx2 >= numLeds) {
                Serial.printf("WavePattern: Index out of range - idx1=%d, idx2=%d, numLeds=%d\n", idx1, idx2, numLeds);
                continue;
            }
            
            // 現在の位置に基づいて明るさを取得
            int brightnessIndex = (i + wavePos) % 8;
            uint8_t brightnessVal = brightnessTable[brightnessIndex];
            
            // デバッグ出力
            Serial.printf("WavePattern: Face %d (idx1=%d, idx2=%d): Brightness %d\n", i, idx1, idx2, brightnessVal);
            
            // 青色をベースに明るさを適用
            CRGB color = CRGB::Blue;
            color.nscale8_video(brightnessVal);
            
            leds[idx1] = color;
            leds[idx2] = color;
        }
        
        FastLED.show();
        
        // 波の位置を更新
        wavePos = (wavePos + 1) % 8;
        
        // 更新頻度
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    Serial.println("WavePattern: Pattern completed");
}

// レインボーパターンの実装
void RainbowPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    uint8_t hue = 0;
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            // 各面に対して hue にオフセットを加える
            CRGB color = CHSV(hue + i * 32, 255, 255);
            leds[idx1] = color;
            leds[idx2] = color;
        }
        FastLED.show();
        hue++;  // hue を徐々に増加させる
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// ストロボパターンの実装
void StrobePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    bool state = false;
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            if (state) {
                leds[idx1] = CRGB::White;
                leds[idx2] = CRGB::White;
            } else {
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
        }
        FastLED.show();
        state = !state;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// チェイスパターンの実装
void ChasePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            // まず全面を消灯
            for (int j = 0; j < numFaces; j++) {
                int idx1 = ledOffset + (j * 2);
                int idx2 = ledOffset + (j * 2) + 1;
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
            // face i を点灯
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            leds[idx1] = CRGB::White;
            leds[idx2] = CRGB::White;
            FastLED.show();
            vTaskDelay(300 / portTICK_PERIOD_MS);
        }
    }
}

// パルスパターンの実装
void PulsePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        // 明るくするフェーズ
        for (uint8_t b = 0; b < 255; b += 5) {
            for (int i = 0; i < numFaces; i++) {
                int idx1 = ledOffset + (i * 2);
                int idx2 = ledOffset + (i * 2) + 1;
                leds[idx1] = CRGB::White;
                leds[idx2] = CRGB::White;
                leds[idx1].nscale8_video(b);
                leds[idx2].nscale8_video(b);
            }
            FastLED.show();
            vTaskDelay(30 / portTICK_PERIOD_MS);
        }
        // 暗くするフェーズ
        for (int b = 255; b > 0; b -= 5) {
            for (int i = 0; i < numFaces; i++) {
                int idx1 = ledOffset + (i * 2);
                int idx2 = ledOffset + (i * 2) + 1;
                leds[idx1] = CRGB::White;
                leds[idx2] = CRGB::White;
                leds[idx1].nscale8_video(b);
                leds[idx2].nscale8_video(b);
            }
            FastLED.show();
            vTaskDelay(30 / portTICK_PERIOD_MS);
        }
    }
}

// ツインクルパターンの実装
void TwinklePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            // 50%の確率でツインクル
            if (random(100) < 50) {
                uint8_t bright = random(50, 255);
                CRGB color = CRGB::White;
                color.nscale8_video(bright);
                leds[idx1] = color;
                leds[idx2] = color;
            } else {
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
        }
        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// 炎のちらつきパターンの実装
void FireFlickerPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            // flicker 値で明るさをランダムに決定
            uint8_t flicker = random(100, 255);
            // 炎っぽさを出すため、赤を主体に、緑は flicker の 0～値の一部、青はゼロ
            CRGB color = CRGB(flicker, random(0, flicker / 2), 0);
            leds[idx1] = color;
            leds[idx2] = color;
        }
        FastLED.show();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// コメットパターンの実装
void CometPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    // まず全LEDを消灯
    for (int i = ledOffset; i < numLeds; i++) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
    unsigned long startTime = millis();
    int cometPos = 0;
    while (millis() - startTime < duration) {
        // 各ループごとに全LEDを少しフェードさせる
        for (int i = ledOffset; i < numLeds; i++) {
            leds[i].nscale8_video(200); // 約20%程度の減衰（調整可能）
        }
        // 現在のコメット位置の面を白色で点灯
        int idx1 = ledOffset + (cometPos * 2);
        int idx2 = ledOffset + (cometPos * 2) + 1;
        leds[idx1] = CRGB::White;
        leds[idx2] = CRGB::White;
        
        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        cometPos = (cometPos + 1) % numFaces;
    }
}

// 個別ランダムパターンの実装
void IndividualRandomPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            // 50%の確率でランダムな色にする、そうでなければ消灯
            if (random(100) < 50) {
                CRGB randColor = CRGB(random(256), random(256), random(256));
                leds[idx1] = randColor;
                leds[idx2] = randColor;
            } else {
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
        }
        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

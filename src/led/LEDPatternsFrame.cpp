#include "LEDManager.h"

// SequentialPatternのフレームベース実装
void SequentialPattern::runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
    // 初回フレームの場合は初期化
    if (m_isFirstFrame) {
        m_patternStartTime = millis();
        m_currentStep = 0;
        m_isFirstFrame = false;
        m_lastStepTime = millis();
    }
    
    // ステップ間の時間（1秒）が経過したら次のステップへ
    unsigned long currentTime = millis();
    if (currentTime - m_lastStepTime >= 1000) {
        m_currentStep = (m_currentStep + 1) % numFaces;
        m_lastStepTime = currentTime;
    }
    
    // 現在のステップに基づいてLEDを更新
    for (int j = 0; j < numFaces; j++) {
        int idx1 = ledOffset + (j * 2);
        int idx2 = ledOffset + (j * 2) + 1;
        if (j <= m_currentStep) {
            leds[idx1] = CRGB::White;
            leds[idx2] = CRGB::White;
        } else {
            leds[idx1] = CRGB::Black;
            leds[idx2] = CRGB::Black;
        }
    }
    FastLED.show();
}

// RainbowPatternのフレームベース実装
void RainbowPattern::runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
    // 初回フレームの場合は初期化
    if (m_isFirstFrame) {
        m_patternStartTime = millis();
        m_currentStep = 0; // hue値として使用
        m_isFirstFrame = false;
    }
    
    // 各面に対してhueを適用
    for (int i = 0; i < numFaces; i++) {
        int idx1 = ledOffset + (i * 2);
        int idx2 = ledOffset + (i * 2) + 1;
        // 各面に対して hue にオフセットを加える
        CRGB color = CHSV(m_currentStep + i * 32, 255, 255);
        leds[idx1] = color;
        leds[idx2] = color;
    }
    FastLED.show();
    
    // hueを徐々に増加
    m_currentStep = (m_currentStep + 1) % 256;
}

// OnOffPatternのフレームベース実装
void OnOffPattern::runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
    // 初回フレームの場合は初期化
    if (m_isFirstFrame) {
        m_patternStartTime = millis();
        m_currentStep = 0; // 0=OFF, 1=ON
        m_isFirstFrame = false;
        m_lastStepTime = millis();
    }
    
    // 状態切り替えの時間（1秒）が経過したら状態を切り替え
    unsigned long currentTime = millis();
    if (currentTime - m_lastStepTime >= 1000) {
        m_currentStep = (m_currentStep + 1) % 2; // 0と1を交互に
        m_lastStepTime = currentTime;
    }
    
    // 現在の状態に基づいてLEDを更新
    for (int i = 0; i < numFaces; i++) {
        int idx1 = ledOffset + (i * 2);
        int idx2 = ledOffset + (i * 2) + 1;
        if (m_currentStep == 1) { // ON状態
            leds[idx1] = CRGB::White;
            leds[idx2] = CRGB::White;
        } else { // OFF状態
            leds[idx1] = CRGB::Black;
            leds[idx2] = CRGB::Black;
        }
    }
    FastLED.show();
}

// StrobePatternのフレームベース実装
void StrobePattern::runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
    // 初回フレームの場合は初期化
    if (m_isFirstFrame) {
        m_patternStartTime = millis();
        m_currentStep = 0; // 0=OFF, 1=ON
        m_isFirstFrame = false;
        m_lastStepTime = millis();
    }
    
    // 状態切り替えの時間（100ms）が経過したら状態を切り替え
    unsigned long currentTime = millis();
    if (currentTime - m_lastStepTime >= 100) {
        m_currentStep = (m_currentStep + 1) % 2; // 0と1を交互に
        m_lastStepTime = currentTime;
    }
    
    // 現在の状態に基づいてLEDを更新
    for (int i = 0; i < numFaces; i++) {
        int idx1 = ledOffset + (i * 2);
        int idx2 = ledOffset + (i * 2) + 1;
        if (m_currentStep == 1) { // ON状態
            leds[idx1] = CRGB::White;
            leds[idx2] = CRGB::White;
        } else { // OFF状態
            leds[idx1] = CRGB::Black;
            leds[idx2] = CRGB::Black;
        }
    }
    FastLED.show();
}

// PulsePatternのフレームベース実装
void PulsePattern::runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
    // 初回フレームの場合は初期化
    if (m_isFirstFrame) {
        m_patternStartTime = millis();
        m_currentStep = 0; // 明るさ値として使用
        m_isFirstFrame = false;
        m_pulseDirection = 1; // 1=明るくする, -1=暗くする
    }
    
    // 明るさを更新
    if (m_pulseDirection > 0) {
        m_currentStep += 5;
        if (m_currentStep >= 255) {
            m_currentStep = 255;
            m_pulseDirection = -1;
        }
    } else {
        m_currentStep -= 5;
        if (m_currentStep <= 0) {
            m_currentStep = 0;
            m_pulseDirection = 1;
        }
    }
    
    // 現在の明るさに基づいてLEDを更新
    for (int i = 0; i < numFaces; i++) {
        int idx1 = ledOffset + (i * 2);
        int idx2 = ledOffset + (i * 2) + 1;
        leds[idx1] = CRGB::White;
        leds[idx2] = CRGB::White;
        leds[idx1].nscale8_video(m_currentStep);
        leds[idx2].nscale8_video(m_currentStep);
    }
    FastLED.show();
}

// FireFlickerPatternのフレームベース実装
void FireFlickerPattern::runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
    // 初回フレームの場合は初期化
    if (m_isFirstFrame) {
        m_patternStartTime = millis();
        m_isFirstFrame = false;
    }
    
    // 各面に対してランダムなちらつきを適用
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
}

// FpsTestPatternの実装
void FpsTestPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    // 初期化
    m_lastLogTime = millis();
    m_frameCounter = 0;
    m_hue = 0;
    
    // 親クラスのrun()メソッドを呼び出す（runFrame()を使用）
    LedPattern::run(leds, numLeds, ledOffset, numFaces, duration);
}

// FpsTestPatternのフレームベース実装
void FpsTestPattern::runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
    // 初回フレームの場合は初期化
    if (m_isFirstFrame) {
        m_patternStartTime = millis();
        m_lastLogTime = millis();
        m_frameCounter = 0;
        m_hue = 0;
        m_isFirstFrame = false;
        
        // 初期状態をログ出力
        Serial.println("FpsTestPattern: Starting FPS test");
        Serial.println("FpsTestPattern: This pattern will show a rainbow effect with FPS logging");
    }
    
    // フレームカウンターを更新
    m_frameCounter++;
    
    // 1秒ごとにFPS情報をログ出力
    unsigned long currentTime = millis();
    if (currentTime - m_lastLogTime >= 1000) {
        float fps = m_frameCounter * 1000.0f / (currentTime - m_lastLogTime);
        Serial.printf("FpsTestPattern: FPS = %.2f (frames: %d, time: %lu ms)\n", 
                     fps, m_frameCounter, currentTime - m_lastLogTime);
        m_lastLogTime = currentTime;
        m_frameCounter = 0;
    }
    
    // 各面に対して色相を変化させながら色を適用
    for (int i = 0; i < numFaces; i++) {
        int idx1 = ledOffset + (i * 2);
        int idx2 = ledOffset + (i * 2) + 1;
        
        // 各面に異なる色相を適用（色相環を一周）
        uint8_t faceHue = m_hue + (i * 256 / numFaces);
        CRGB color = CHSV(faceHue, 255, 255);
        
        leds[idx1] = color;
        leds[idx2] = color;
    }
    
    // 色相を徐々に変化させる
    m_hue++;
    
    // LEDを更新
    FastLED.show();
}

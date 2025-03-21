#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include "Constants.h"
#include "JsonLEDPatterns.h"

// FPS制御クラス
class FpsController {
private:
    uint32_t m_targetFrameTime; // 目標フレーム時間（マイクロ秒）
    uint32_t m_lastFrameTime;   // 前回フレーム開始時間
    uint32_t m_actualFps;       // 実際のFPS（モニタリング用）
    uint32_t m_frameCount;      // フレームカウンター
    uint32_t m_fpsUpdateTime;   // FPS計算用タイムスタンプ
    
public:
    FpsController(uint16_t targetFps = 30) {
        setTargetFps(targetFps);
        m_lastFrameTime = 0;
        m_actualFps = 0;
        m_frameCount = 0;
        m_fpsUpdateTime = 0;
    }
    
    void setTargetFps(uint16_t fps) {
        // 0除算防止
        if (fps < 1) fps = 1;
        // 1秒 = 1,000,000マイクロ秒
        m_targetFrameTime = 1000000 / fps;
    }
    
    uint16_t getActualFps() const {
        return m_actualFps;
    }
    
    // フレーム開始時に呼び出す
    void beginFrame() {
        m_lastFrameTime = micros();
    }
    
    // フレーム終了時に呼び出す（必要な遅延を自動適用）
    void endFrame() {
        // フレームカウント更新
        m_frameCount++;
        
        // 1秒ごとに実際のFPSを計算
        uint32_t currentTime = micros();
        if (currentTime - m_fpsUpdateTime >= 1000000) {
            m_actualFps = m_frameCount;
            m_frameCount = 0;
            m_fpsUpdateTime = currentTime;
        }
        
        // 経過時間を計算
        uint32_t elapsedTime = micros() - m_lastFrameTime;
        
        // 目標フレーム時間より処理が短い場合は遅延を追加
        if (elapsedTime < m_targetFrameTime) {
            uint32_t delayTime = m_targetFrameTime - elapsedTime;
            
            // マイクロ秒をミリ秒に変換（vTaskDelayはミリ秒単位）
            uint32_t delayMs = delayTime / 1000;
            
            // 最小遅延時間（1ms）未満の場合は微調整
            if (delayMs < 1) {
                // マイクロ秒単位の短い遅延にはdelayMicroseconds()を使用
                // ただしFreeRTOSでは非推奨なので、代わりにNOP命令を使用
                uint32_t nopCount = delayTime / 10; // 約10マイクロ秒ごとにNOP
                for (uint32_t i = 0; i < nopCount; i++) {
                    asm volatile ("nop");
                }
            } else {
                // 1ms以上の遅延はvTaskDelayを使用
                vTaskDelay(delayMs / portTICK_PERIOD_MS);
            }
        }
        // 注: 処理が目標時間を超えた場合は遅延なし（フレームドロップ）
    }
};

// LEDパターンの抽象基底クラス
class LedPattern {
protected:
    // パターン状態の保持用変数
    unsigned long m_patternStartTime;
    int m_currentStep;
    bool m_isFirstFrame;
    
public:
    LedPattern() : m_currentStep(0), m_isFirstFrame(true) {}
    
    // 従来のrun()メソッド（下位互換性のため維持）
    virtual void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
        m_patternStartTime = millis();
        m_currentStep = 0;
        m_isFirstFrame = true;
        
        while (millis() - m_patternStartTime < (unsigned long)duration || duration == 0) {
            runFrame(leds, numLeds, ledOffset, numFaces);
            // 従来の実装では各パターンが独自に遅延を管理
            vTaskDelay(50 / portTICK_PERIOD_MS); // デフォルト遅延
        }
    }
    
    // 新しいフレームベースのメソッド（FPS制御用）
    virtual void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
        // 初回フレームの場合は初期化
        if (m_isFirstFrame) {
            m_patternStartTime = millis();
            m_isFirstFrame = false;
        }
        
        // デフォルト実装（派生クラスでオーバーライド）
        // 単純に全LEDを消灯
        for (int i = 0; i < numLeds; i++) {
            leds[i] = CRGB::Black;
        }
        FastLED.show();
    }
    
    virtual void reset() {
        m_currentStep = 0;
        m_isFirstFrame = true;
    }
    
    virtual String getName() = 0;
    virtual ~LedPattern() {}
};

// 各パターンクラスはLedPatternを継承して実装
class SequentialPattern : public LedPattern {
private:
    unsigned long m_lastStepTime;
    
public:
    SequentialPattern() : m_lastStepTime(0) {}
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) override;
    String getName() override { return "Sequential"; }
};

class OnOffPattern : public LedPattern {
private:
    unsigned long m_lastStepTime;
    
public:
    OnOffPattern() : m_lastStepTime(0) {}
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) override;
    String getName() override { return "On/Off"; }
};

class OddEvenPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Odd/Even"; }
};

class RandomPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Random"; }
};

class WavePattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Wave"; }
};

class RainbowPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) override;
    String getName() override { return "Rainbow"; }
};

class StrobePattern : public LedPattern {
private:
    unsigned long m_lastStepTime;
    
public:
    StrobePattern() : m_lastStepTime(0) {}
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) override;
    String getName() override { return "Strobe"; }
};

class ChasePattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Chase"; }
};

class PulsePattern : public LedPattern {
private:
    int m_pulseDirection;
    
public:
    PulsePattern() : m_pulseDirection(1) {}
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) override;
    String getName() override { return "Pulse"; }
};

class TwinklePattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Twinkle"; }
};

class FireFlickerPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) override;
    String getName() override { return "FireFlicker"; }
};

class CometPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Comet"; }
};

class IndividualRandomPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Individual Random"; }
};

// FPS管理テスト用のパターン
class FpsTestPattern : public LedPattern {
private:
    unsigned long m_lastLogTime;
    int m_frameCounter;
    uint8_t m_hue;
    
public:
    FpsTestPattern() : m_lastLogTime(0), m_frameCounter(0), m_hue(0) {}
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    void runFrame(CRGB* leds, int numLeds, int ledOffset, int numFaces) override;
    String getName() override { return "FPS Test"; }
};

class LEDManager {
private:
    CRGB* leds;
    int numLeds;
    int ledOffset;
    int numFaces;
    LedPattern** patterns;
    int patternCount;
    int currentPatternIndex;
    TaskHandle_t ledTaskHandle;
    uint8_t brightness;
    bool isTaskRunning;  // タスクが実行中かどうかを追跡するフラグ
    
    // FPS制御関連
    FpsController m_fpsController;
    uint16_t m_targetFps;
    bool m_fpsControlEnabled;
    
    // JSONパターン関連
    JsonPatternManager m_jsonPatternManager;
    bool m_isJsonPattern;  // 現在実行中のパターンがJSONパターンかどうか
    int m_currentJsonPatternIndex;  // 現在実行中のJSONパターンのインデックス
    
    static void ledTaskWrapper(void* parameter);
    static void jsonPatternTaskWrapper(void* parameter);

public:
    LEDManager();
    ~LEDManager();
    void begin(int pin, int numLeds, int ledOffset);
    void runPattern(int patternIndex);
    void stopPattern();
    void lightFace(int faceId, CRGB color);
    CRGB getFaceColor(int faceId);
    void resetAllLeds();
    int getPatternCount() { return patternCount; }
    void nextPattern();
    void prevPattern();
    String getPatternName(int index);
    String getCurrentPatternName();
    int getCurrentPatternIndex();
    void setBrightness(uint8_t brightness);
    bool isPatternRunning();
    
    // FPS制御関連のメソッド
    void setTargetFps(uint16_t fps);
    uint16_t getTargetFps() const;
    uint16_t getActualFps() const;
    void enableFpsControl(bool enable);
    bool isFpsControlEnabled() const;
    
    // JSONパターン関連のメソッド
    bool loadJsonPatternsFromFile(const String& filename);
    bool loadJsonPatternsFromString(const String& jsonString);
    bool loadJsonPatternsFromDirectory(const String& dirPath);
    int getJsonPatternCount();
    String getJsonPatternName(int index);
    void runJsonPattern(const String& patternName);
    void runJsonPatternByIndex(int index);
    bool isJsonPatternRunning() { return m_isJsonPattern && isPatternRunning(); }
    
    // 受信したJSONパターンを実行するメソッド
    bool runJsonPatternFromFile(const String& filename);
    
    // ゲッターメソッド
    CRGB* getLeds() { return leds; }
    int getNumLeds() { return numLeds; }
    int getLedOffset() { return ledOffset; }
    int getNumFaces() { return numFaces; }
};

#endif // LED_MANAGER_H

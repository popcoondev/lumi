#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include "Constants.h"
#include "JsonLEDPatterns.h"

// LEDパターンの抽象基底クラス
class LedPattern {
public:
    virtual void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) = 0;
    virtual String getName() = 0;
    virtual ~LedPattern() {}
};

// 各パターンクラスはLedPatternを継承して実装
class SequentialPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Sequential"; }
};

class OnOffPattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
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
    String getName() override { return "Rainbow"; }
};

class StrobePattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Strobe"; }
};

class ChasePattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
    String getName() override { return "Chase"; }
};

class PulsePattern : public LedPattern {
public:
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override;
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

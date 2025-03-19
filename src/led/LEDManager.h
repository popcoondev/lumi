#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include <FastLED.h>
#include "Constants.h"

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
    
    static void ledTaskWrapper(void* parameter);

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
};

#endif // LED_MANAGER_H

#ifndef JSON_LED_PATTERN_WRAPPER_H
#define JSON_LED_PATTERN_WRAPPER_H

#include "LEDManager.h"
#include "JsonLEDPatterns.h"

// LedPatternを継承し、JsonLedPatternをラップするクラス
class JsonLedPatternWrapper : public LedPattern {
public:
    JsonLedPatternWrapper(JsonLedPattern* pattern) : m_pattern(pattern) {}
    
    ~JsonLedPatternWrapper() {
        // パターンの解放はJsonPatternManagerが行うため、ここでは何もしない
    }
    
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) override {
        if (m_pattern) {
            m_pattern->run(leds, numLeds, ledOffset, numFaces, duration);
        }
    }
    
    String getName() override {
        if (m_pattern) {
            return m_pattern->getName();
        }
        return "Unknown";
    }
    
private:
    JsonLedPattern* m_pattern;
};

#endif // JSON_LED_PATTERN_WRAPPER_H

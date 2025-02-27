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

// RandomPattern, WavePattern, RainbowPattern, StrobePattern, 
void RandomPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            if (random(100) < 50) {
                uint8_t bright = random(50, 255);
                CRGB color = CRGB::White;
                color.nscale8_video(bright);
                leds[i] = color;
            } else {
                leds[i] = CRGB::Black;
            }
        }
        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void WavePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            float phase = (millis() / 100.0) + (i * PI / 4);
            uint8_t brightnessVal = (uint8_t)(((sin(phase) + 1.0) / 2.0) * 255);
            CRGB baseColor = CRGB::Blue;
            CRGB modulatedColor = baseColor;
            modulatedColor.nscale8_video(brightnessVal);
            leds[i] = modulatedColor;
        }
        FastLED.show();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void RainbowPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    uint8_t hue = 0;
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            CRGB color = CHSV(hue, 255, 255);
            leds[i] = color;
        }
        FastLED.show();
        hue++;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void StrobePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            if (i % 2 == 0) {
                leds[i] = CRGB::White;
            } else {
                leds[i] = CRGB::Black;
            }
        }
        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// ChasePattern, PulsePattern, TwinklePattern, FireFlickerPattern,
void ChasePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            if (i % 2 == 1) {
                leds[i] = CRGB::White;
            } else {
                leds[i] = CRGB::Black;
            }
        }
        FastLED.show();
        vTaskDelay(1000);
    }
}

void PulsePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            float phase = (millis() / 100.0) + (i * M_PI / 4);
            uint8_t brightnessVal = (uint8_t)(((sin(phase) + 1.0) / 2.0) * 255);
            CRGB baseColor = CRGB::Blue;
            CRGB modulatedColor = baseColor;
            modulatedColor.nscale8_video(brightnessVal);
            leds[i] = modulatedColor;
        }
        FastLED.show();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void TwinklePattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            if (random(100) < 50) {
                uint8_t bright = random(50, 255);
                CRGB color = CRGB::White;
                color.nscale8_video(bright);
                leds[i] = color;
            } else {
                leds[i] = CRGB::Black;
            }
        }
        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void FireFlickerPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            if (i % 2 == 0) {
                leds[i] = CRGB::White;
            } else {
                leds[i] = CRGB::Black;
            }
        }
        FastLED.show();
        vTaskDelay(1000);
    }
}

// CometPattern, IndividualRandomPattern
void CometPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    int cometPos = 0;
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            if (i % 2 == 0) {
                int idx1 = ledOffset + (cometPos * 2);
                int idx2 = ledOffset + (cometPos * 2) + 1;
                leds[idx1] = CRGB::White;
                leds[idx2] = CRGB::White;
            }
        }
        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        cometPos = (cometPos + 1) % (numFaces / 2);
    }
}

void IndividualRandomPattern::run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        for (int i = 0; i < numFaces; i++) {
            CRGB randColor = CRGB(random(256), random(256), random(256));
            leds[i] = randColor;
        }
        FastLED.show();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
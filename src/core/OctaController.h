#ifndef OCTA_CONTROLLER_H
#define OCTA_CONTROLLER_H

#include <Arduino.h>
#include <M5Unified.h>
#include "../ui/UIManager.h"
#include "../led/LEDManager.h"
#include "../system/FaceDetector.h"
#include "../system/StateManager.h"
#include "ButtonEvent.h"
#include "../ui/views/LumiView.h"
#include "../mic/MicManager.h"

// フレームレート制御のための定数
#define TARGET_FPS 30
#define FRAME_TIME (1000 / TARGET_FPS) // ms

// IMUセンサー更新間隔
#define IMU_UPDATE_INTERVAL 50 // ms

class OctaController {
private:
    UIManager* uiManager;
    LEDManager* ledManager;
    IMUSensor* imuSensor;
    FaceDetector* faceDetector;
    StateManager* stateManager;
    LumiView* lumiView;
    MicManager* micManager;
    std::function<void(int)> micCallback;
    
    unsigned long lastUpdateTime; // 最後の更新時間
    CRGB currentLedColor;         // 現在選択されている色
    uint8_t currentHue;           // 現在の色相値 (0-255)
    uint8_t currentSaturation;    // 現在の彩度値 (0-255)
    uint8_t currentValueBrightness; // 現在の明度値 (0-255)
    
    // CRGB色をM5Stack LCD用のuint16_t色に変換する関数
    uint16_t crgbToRGB565(CRGB color);
    
    void handleButtonEvent(ButtonEvent event);
    void processLumiHomeState();
    void processDetectionState();
    void processCalibrationState();
    void processLEDControlState();
    void processLedControlButtons(ButtonEvent event);

public:
    OctaController();
    ~OctaController();
    void setup();
    void loop();
    void LumiHomeSetInitialDraw();
    int mapViewFaceToLedFace(int viewFaceId);
    int mapLedFaceToViewFace(int ledFaceId);
};

#endif // OCTA_CONTROLLER_H

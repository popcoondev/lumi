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
#include "../activities/LumiHomeActivity.h"
#include "../activities/SplashActivity.h"
#include "../activities/SettingsActivity.h"
#include "../activities/DetectionActivity.h"
#include "../activities/CalibrationActivity.h"
#include "../activities/LEDControlActivity.h"
#include "../activities/NetworkSettingsActivity.h"
#include "../framework/ActivityManager.h"
#include "../network/NetworkManager.h"
#include "../network/WebServerManager.h"

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
    framework::ActivityManager* activityManager;
    LumiView* lumiView;
    LumiHomeActivity* lumiHomeActivity;
    SplashActivity* splashActivity;
    SettingsActivity* settingsActivity;
    DetectionActivity* detectionActivity;
    CalibrationActivity* calibrationActivity;
    LEDControlActivity* ledControlActivity;
    NetworkSettingsActivity* networkSettingsActivity;
    MicManager* micManager;
    NetworkManager* networkManager;
    WebServerManager* webServerManager;
    std::function<void(const std::array<double, 8>&, double)> micCallback;

    
    unsigned long lastUpdateTime; // 最後の更新時間
    CRGB currentLedColor;         // 現在選択されている色
    uint8_t currentHue;           // 現在の色相値 (0-255)
    uint8_t currentSaturation;    // 現在の彩度値 (0-255)
    uint8_t currentValueBrightness; // 現在の明度値 (0-255)
    
    // CRGB色をM5Stack LCD用のuint16_t色に変換する関数
    uint16_t crgbToRGB565(CRGB color);
    
    void handleButtonEvent(ButtonEvent event);
    void processLumiHomeState();
    void processSettingsState();
    void processDetectionActivityState();
    void processCalibrationActivityState();
    void processLEDControlActivityState();
    void processNetworkSettingsActivityState();
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

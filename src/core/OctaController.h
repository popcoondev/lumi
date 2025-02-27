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
    
    unsigned long lastUpdateTime; // 最後の更新時間
    
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
};

#endif // OCTA_CONTROLLER_H
#ifndef OCTA_CONTROLLER_H
#define OCTA_CONTROLLER_H

#include <Arduino.h>
#include <M5Unified.h>
#include "UIManager.h"
#include "LEDManager.h"
#include "FaceDetector.h"
#include "StateManager.h"
#include "ButtonEvent.h"

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
    
    unsigned long lastUpdateTime; // 最後の更新時間
    
    void handleButtonEvent(ButtonEvent event);
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
#include <Arduino.h>
#include <M5Unified.h>
#include "Toolbar.h"
#include <FastLED.h>
#include "TextView.h"
#include "ActionBar.h"

ActionBar actionBar;
TextView mainTextView;
TextView subTextView;
Toolbar toolbar;

// センサーしきい値
#define STABLE_THRESHOLD 0.2 
#define STABLE_DURATION 1000 
#define TIMEOUT_DURATION 120000

// LED設定
#define LED_PIN 8    
#define NUM_LEDS 30  
uint8_t brightness = 255;  
bool ledState = false;
CRGB leds[NUM_LEDS];

// IMUデータ
float accX, accY, accZ;

// Faceデータ
struct FaceData {
    int id;
    float x, y, z;
};
FaceData faceList[20];  
int calibratedFaces = 0;
int count = 0;
// システム状態
enum State { STATE_NONE, STATE_DETECTION, STATE_CALIBRATION, STATE_LED_CONTROL };
State currentState = STATE_NONE;

// キャリブレーションのサブステート
enum State_calibration { 
    STATE_CALIBRATION_INIT, 
    STATE_CALIBRATION_WAIT_STABLE, 
    STATE_CALIBRATION_DETECT_FACE, 
    STATE_CALIBRATION_CONFIRM_NEW_FACE,
    STATE_CALIBRATION_COMPLETE
};
State_calibration calibrationState = STATE_CALIBRATION_INIT;

// 最も近い面を探す
int getNearestFace(float x, float y, float z) {
    int closestFace = -1;
    float minDistance = 9999;
    for (int i = 0; i < calibratedFaces; i++) {
        float dx = faceList[i].x - x;
        float dy = faceList[i].y - y;
        float dz = faceList[i].z - z;
        float distance = sqrt(dx * dx + dy * dy + dz * dz);
        if (distance < minDistance) {
            minDistance = distance;
            closestFace = faceList[i].id;
        }
    }
    return closestFace;
}

// キャリブレーションの処理
void processCalibrationState() {
    switch (calibrationState) {
        case STATE_CALIBRATION_INIT:
            mainTextView.setText("Calibrating... Hold still.");
            calibrationState = STATE_CALIBRATION_WAIT_STABLE;
            // toolbar を更新, 
            break;

        case STATE_CALIBRATION_WAIT_STABLE:
            M5.Imu.getAccel(&accX, &accY, &accZ);
            // 安定状態を判定 (簡単な例: 一定時間変化なし)
            static unsigned long stableStartTime = millis();
            if (millis() - stableStartTime > STABLE_DURATION) {
                calibrationState = STATE_CALIBRATION_DETECT_FACE;
            }
            break;

        case STATE_CALIBRATION_DETECT_FACE:
            M5.Imu.getAccel(&accX, &accY, &accZ);
            float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
            float normX = accX / mag;
            float normY = accY / mag;
            float normZ = accZ / mag;
            int detectedFace = getNearestFace(normX, normY, normZ);
            if (detectedFace == -1) {
                mainTextView.setText("Detect new face. Add?");
                toolbar.setButtonLabel(BTN_A, "OK");
                toolbar.setButtonLabel(BTN_B, "Cancel");
                calibrationState = STATE_CALIBRATION_CONFIRM_NEW_FACE;
            } else {
                mainTextView.setText(String("Detected Face: ") + detectedFace);
                calibrationState = STATE_CALIBRATION_COMPLETE;
            }
            break;

    }
}

// メインループの処理
void processState() {
    switch (currentState) {
        case STATE_DETECTION:
            mainTextView.setText("Detecting Mode");
            break;
        case STATE_CALIBRATION:
            mainTextView.setText("Calibration Mode");
            break;
        case STATE_LED_CONTROL:
            mainTextView.setText("LED Control Mode");
            break;
        default:
            break;
    }
}

// ステート変更
void changeState(State newState) {
    currentState = newState;
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    actionBar.begin();
    actionBar.setTitle("Main Menu");
    actionBar.setStatus("Ready");

    mainTextView.begin();
    mainTextView.setPosition(0, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3*2, SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT);
    mainTextView.setFontSize(2);
    mainTextView.setColor(WHITE);
    mainTextView.setBackgroundColor(BLACK);
    mainTextView.setText("System Ready");
    mainTextView.draw();

    toolbar.begin();
    toolbar.setButtonLabel(BTN_A, "Detect");
    toolbar.setButtonLabel(BTN_B, "Calib");
    toolbar.setButtonLabel(BTN_C, "LED");
    toolbar.draw();
}

void loop() {
    if (actionBar.isBackPressed()) {
        Serial.println("Back button pressed!");
        currentState = STATE_NONE;
        actionBar.setTitle("Main Menu");
        actionBar.setStatus(String(count));
        count++;
    }

    if (toolbar.getPressedButton() == BTN_A) changeState(STATE_DETECTION);
    if (toolbar.getPressedButton() == BTN_B) changeState(STATE_CALIBRATION);
    if (toolbar.getPressedButton() == BTN_C) changeState(STATE_LED_CONTROL);
    processState();
    delay(200);
}

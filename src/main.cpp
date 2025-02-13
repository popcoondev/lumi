#include <Arduino.h>
#include <M5Unified.h>
#include "Toolbar.h"
#include <FastLED.h>
#include "TextView.h"
#include "ActionBar.h"
#include "Dialog.h"

ActionBar actionBar;
TextView mainTextView;
TextView subTextView;
Toolbar toolbar;
Dialog dialog;

// センサーしきい値
#define STABLE_THRESHOLD 0.2 
#define STABLE_DURATION 1000 * 5
#define TIMEOUT_DURATION 120000
bool isViewUpdate = false;

// LED設定
#define LED_PIN 8    
#define NUM_LEDS 30  
uint8_t brightness = 255;  
bool ledState = false;
CRGB leds[NUM_LEDS];

// IMUデータ
float accX, accY, accZ;

// 前回値
float prevAccX, prevAccY, prevAccZ;

// Faceデータ
struct FaceData {
    int id;
    float x, y, z;
};
FaceData faceList[20];  
int calibratedFaces = 0;
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

// 内積を利用して最も近い面を判定
int getNearestFace(float x, float y, float z) {
    int nearestFace = -1;
    float maxSimilarity = -1;  // 内積の最大値（-1から1の範囲）

    for (int i = 0; i < calibratedFaces; i++) {
        // 正規化された法線ベクトル
        float Nx = faceList[i].x;
        float Ny = faceList[i].y;
        float Nz = faceList[i].z;

        // 内積を計算（cosθに相当）
        float dotProduct = x * Nx + y * Ny + z * Nz;

        // 内積が最大（最も類似した面）を採用
        if (dotProduct > maxSimilarity) {
            maxSimilarity = dotProduct;
            nearestFace = faceList[i].id;
        }
    }

    // cosθ（dotProduct）がしきい値以上の場合のみ採用
    if (maxSimilarity > 0.95) {  // 0.95は誤差の許容範囲
        return nearestFace;
    } else {
        return -1;
    }
}

// キャリブレーションの処理
void processCalibrationState() {
    float mag, normX, normY, normZ;
    int detectedFace = -1;  // 事前に初期化

    switch (calibrationState) {
        case STATE_CALIBRATION_INIT:
            mainTextView.setText("STATE_CALIBRATION_INIT");
            calibrationState = STATE_CALIBRATION_WAIT_STABLE;
            
            // 前回値のリセット
            prevAccX = 0;
            prevAccY = 0;
            prevAccZ = 0;

            // toolBarの更新
            toolbar.setButtonLabel(BTN_A, "FINISH");
            break;

        case STATE_CALIBRATION_WAIT_STABLE:
            mainTextView.setText("STATE_CALIBRATION_WAIT_STABLE\n" + String(calibratedFaces) + " faces calibrated");
            M5.Imu.getAccel(&accX, &accY, &accZ);
            static unsigned long stableStartTime = millis();
            subTextView.setText("x=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ)
                + "\n" + String(millis() - stableStartTime) + "ms");

            if (abs(accX - prevAccX) < STABLE_THRESHOLD &&
                abs(accY - prevAccY) < STABLE_THRESHOLD &&
                abs(accZ - prevAccZ) < STABLE_THRESHOLD) {
                if (millis() - stableStartTime > STABLE_DURATION) {
                    calibrationState = STATE_CALIBRATION_DETECT_FACE;
                }
            } else {
                stableStartTime = millis();
            }

            prevAccX = accX;
            prevAccY = accY;
            prevAccZ = accZ;
            break;

        case STATE_CALIBRATION_DETECT_FACE:
            mainTextView.setText("STATE_CALIBRATION_DETECT_FACE\n" + String(calibratedFaces) + " faces calibrated");
            M5.Imu.getAccel(&accX, &accY, &accZ);
            subTextView.setText("x=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ));

            mag = sqrt(accX * accX + accY * accY + accZ * accZ);
            normX = accX / mag;
            normY = accY / mag;
            normZ = accZ / mag;
            detectedFace = getNearestFace(normX, normY, normZ);

            if (detectedFace != -1) {
                String message = "Detected existing face: " + String(detectedFace) + "\n detect value: " + String(normX) + ", " + String(normY) + ", " + String(normZ);
                dialog.showDialog("Info", message, DIALOG_OK);
                DialogResult dialogResult = DIALOG_NONE;
                while (dialogResult == DIALOG_NONE) {
                    dialogResult = dialog.getResult();
                    delay(100);
                }
                if (dialogResult == DIALOG_OK_PRESSED) {
                    calibrationState = STATE_CALIBRATION_WAIT_STABLE;
                }
            } else {
                String message = "New face detected. Add to list?\n detect value: " + String(normX) + ", " + String(normY) + ", " + String(normZ);
                dialog.showDialog("Confirm", message, DIALOG_OK_CANCEL);
                DialogResult dialogResult = DIALOG_NONE;
                while (dialogResult == DIALOG_NONE) {
                    dialogResult = dialog.getResult();
                    delay(100);
                }
                if (dialogResult == DIALOG_OK_PRESSED) {
                    Serial.println("DIALOG_OK_PRESSED");
                    faceList[calibratedFaces] = {calibratedFaces + 1, normX, normY, normZ};
                    calibratedFaces++;
                    calibrationState = STATE_CALIBRATION_WAIT_STABLE;
                } else {
                    Serial.println("DIALOG_CANCEL_PRESSED");
                    calibrationState = STATE_CALIBRATION_WAIT_STABLE;
                }
            }

            // 前回値のリセット
            prevAccX = 0;
            prevAccY = 0;
            prevAccZ = 0;

            isViewUpdate = true;
            break;

        case STATE_CALIBRATION_CONFIRM_NEW_FACE:
            mainTextView.setText("STATE_CALIBRATION_CONFIRM_NEW_FACE");
            break;

        case STATE_CALIBRATION_COMPLETE:
            mainTextView.setText("STATE_CALIBRATION_COMPLETE");
            break;

        default:
            break;
    }
}

// メインループの処理
void processState() {
    switch (currentState) {
        case STATE_DETECTION:
            actionBar.setTitle("Face Detection");
            break;
        case STATE_CALIBRATION:
            actionBar.setTitle("Face Calibration");
            processCalibrationState();
            break;
        case STATE_LED_CONTROL:
            actionBar.setTitle("LED Control");
            break;
        default:
            break;
    }

}

// ステート変更
void changeState(State newState) {
    Serial.println("Change state: " + String(newState));
    currentState = newState;
    isViewUpdate = true;
    
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    actionBar.begin();
    actionBar.setTitle("Main Menu");
    actionBar.setStatus("Ready");
    actionBar.draw();

    mainTextView.begin();
    mainTextView.setPosition(0, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3*2, SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT);
    mainTextView.setFontSize(2);
    mainTextView.setColor(WHITE);
    mainTextView.setBackgroundColor(BLACK);
    mainTextView.setText("System Ready");
    mainTextView.draw();

    subTextView.begin();
    subTextView.setPosition(SCREEN_WIDTH/3*2, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3, SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT);
    subTextView.setFontSize(1);
    subTextView.setColor(WHITE);
    subTextView.setBackgroundColor(BLACK);
    subTextView.setText("");
    subTextView.draw();

    toolbar.begin();
    toolbar.setButtonLabel(BTN_A, "Detect");
    toolbar.setButtonLabel(BTN_B, "Calib");
    toolbar.setButtonLabel(BTN_C, "LED");
    toolbar.draw();

    prevAccX = 0;
    prevAccY = 0;
    prevAccZ = 0;

}

void loop() {
    if (actionBar.isBackPressed()) {
        Serial.println("Back button pressed!");
        currentState = STATE_NONE;
        calibrationState = STATE_CALIBRATION_INIT;
        actionBar.setTitle("Main Menu");
        actionBar.setStatus("Ready");
        actionBar.draw();
        mainTextView.setText("System Ready");
        subTextView.setText("");
    }

    if (toolbar.getPressedButton(BTN_A)) changeState(STATE_DETECTION);
    if (toolbar.getPressedButton(BTN_B)) changeState(STATE_CALIBRATION);
    if (toolbar.getPressedButton(BTN_C)) changeState(STATE_LED_CONTROL);
    processState();

    if (isViewUpdate) {
        mainTextView.draw();
        actionBar.draw();
        toolbar.draw();
        isViewUpdate = false;
    }
    delay(200);
}

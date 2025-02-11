// #include <Arduino.h>
// #include <M5Unified.h>

// // センサーのしきい値と判定時間
// #define STABLE_THRESHOLD 0.2 // しきい値 (m/s^2)
// #define STABLE_DURATION 1000 // 判定時間 (ms)

// // IMU（MPU6886）データ
// float accX, accY, accZ;

// // 判定用の変数
// int lastFace = -1;
// unsigned long stableStartTime = 0;

// // キャリブレーション用データ（最大20面）
// struct FaceData {
//     int id;
//     float x, y, z; // 重力加速度の理想値
// };
// FaceData faceList[20];  // キャリブレーションした面データ
// int calibratedFaces = 0; // 登録済みの面数

// // システム状態
// enum Mode { DETECTION, CALIBRATION, LED_CONTROL };
// Mode currentMode = DETECTION; // 初期モードは面検出

// // 最も近い面を探す
// int getNearestFace(float x, float y, float z) {
//     int closestFace = -1;
//     float minDistance = 9999;

//     for (int i = 0; i < calibratedFaces; i++) {
//         float dx = faceList[i].x - x;
//         float dy = faceList[i].y - y;
//         float dz = faceList[i].z - z;
//         float distance = sqrt(dx * dx + dy * dy + dz * dz);

//         if (distance < minDistance) {
//             minDistance = distance;
//             closestFace = faceList[i].id;
//         }
//     }
//     return closestFace;
// }

// // 面の判定（機能1）
// void detectFace() {
//     M5.Imu.getAccel(&accX, &accY, &accZ);

//     // 正規化
//     float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
//     float normX = accX / mag;
//     float normY = accY / mag;
//     float normZ = accZ / mag;

//     // どの面に近いか判定
//     int detectedFace = getNearestFace(normX, normY, normZ);

//     // しきい値内で安定しているか判定
//     if (detectedFace != lastFace) {
//         stableStartTime = millis();  // 新しい面が検出されたら時間リセット
//     }

//     if (millis() - stableStartTime > STABLE_DURATION) {
//         // 判定時間内で変わっていなければ確定
//         if (detectedFace != lastFace) {
//             lastFace = detectedFace;
//             M5.Lcd.fillScreen(BLACK);
//             M5.Lcd.setCursor(10, 10);
//             M5.Lcd.printf("Face Up: %d", detectedFace);
//         }
//     }
// }

// // キャリブレーション（機能2）
// void calibrateFace() {
//     M5.Imu.getAccel(&accX, &accY, &accZ);

//     // 正規化
//     float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
//     float normX = accX / mag;
//     float normY = accY / mag;
//     float normZ = accZ / mag;

//     if (M5.Touch.getCount() > 0) {
//         if (calibratedFaces < 20) {
//             faceList[calibratedFaces] = {calibratedFaces, normX, normY, normZ};
//             calibratedFaces++;
//             M5.Lcd.fillScreen(BLACK);
//             M5.Lcd.setCursor(10, 10);
//             M5.Lcd.printf("Face %d Calibrated!", calibratedFaces - 1);
//         } else {
//             M5.Lcd.fillScreen(BLACK);
//             M5.Lcd.setCursor(10, 10);
//             M5.Lcd.println("Calibration Full!");
//         }
//     }
// }

// // LED制御（機能3）
// void controlLEDs() {
//     // 機能1で取得した lastFace に対応するLEDを点灯
//     M5.Lcd.fillScreen(BLACK);
//     M5.Lcd.setCursor(10, 10);
//     M5.Lcd.printf("LED for Face: %d", lastFace);
    
//     // 実際のLED制御処理をここに追加する
// }

// // システムモードの変更（タッチで変更）
// void checkModeChange() {
//     if (M5.Touch.getCount() > 0) {
//         int touchX = M5.Touch.getDetail(0).x; // タッチ位置X座標を取得

//         if (touchX < 106) {
//             currentMode = DETECTION;
//             M5.Lcd.fillScreen(BLACK);
//             M5.Lcd.setCursor(10, 10);
//             M5.Lcd.println("Mode: Face Detection");
//         } else if (touchX < 213) {
//             currentMode = CALIBRATION;
//             M5.Lcd.fillScreen(BLACK);
//             M5.Lcd.setCursor(10, 10);
//             M5.Lcd.println("Mode: Calibration");
//         } else {
//             currentMode = LED_CONTROL;
//             M5.Lcd.fillScreen(BLACK);
//             M5.Lcd.setCursor(10, 10);
//             M5.Lcd.println("Mode: LED Control");
//         }
//     }
// }

// void setup() {
//     M5.begin();
//     M5.Lcd.setTextSize(2);
//     M5.Lcd.println("Initializing...");

//     if (!M5.Imu.begin()) {
//         M5.Lcd.println("IMU Init Failed!");
//         while (1);  // センサーが見つからない場合は停止
//     }

//     M5.Lcd.fillScreen(BLACK);
//     M5.Lcd.setCursor(10, 10);
//     M5.Lcd.println("Mode: Face Detection");
// }

// void loop() {
//     M5.update(); // タッチの更新
//     checkModeChange(); // モード変更を確認

//     switch (currentMode) {
//         case DETECTION:
//             detectFace();
//             break;
//         case CALIBRATION:
//             calibrateFace();
//             break;
//         case LED_CONTROL:
//             controlLEDs();
//             break;
//     }

//     delay(100);
// }

#include <Arduino.h>
#include <M5Unified.h>
#include "Toolbar.h"

Toolbar toolbar;

void setup() {
    M5.begin();
    Serial.begin(115200);
    Serial.println("System Initialized");

    M5.Lcd.setTextSize(2);
    M5.Lcd.fillScreen(BLACK);
    
    // ツールバーを初期化
    toolbar.begin();
    toolbar.setButtonLabel(BTN_A, "Mode1");
    toolbar.setButtonLabel(BTN_B, "Mode2");
    // toolbar.setButtonLabel(BTN_C, "Mode3");
}

void loop() {
    ButtonID pressed = toolbar.getPressedButton();

    if (pressed != BTN_NONE) {
        Serial.printf("Button Pressed: %d\n", pressed);
        
        // 画面をクリアして表示更新
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setTextColor(WHITE); // 文字色を設定
        M5.Lcd.setCursor(40, 100); // 画面中央に配置

        switch (pressed) {
            case BTN_A:
                Serial.println("Mode 1 Selected");
                M5.Lcd.println("Mode 1 Selected");
                break;
            case BTN_B:
                Serial.println("Mode 2 Selected");
                M5.Lcd.println("Mode 2 Selected");
                break;
            case BTN_C:
                Serial.println("Mode 3 Selected");
                M5.Lcd.println("Mode 3 Selected");
                break;
            default:
                break;
        }
        
        delay(1000);  // 1秒表示を維持
        toolbar.draw();  // ツールバーを再描画
    }
}

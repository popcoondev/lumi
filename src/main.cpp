// #include "main.h"
// #include <Arduino.h>
// #include <M5Unified.h>
// #include "Toolbar.h"
// #include <FastLED.h>
// #include "TextView.h"
// #include "ActionBar.h"
// #include "Dialog.h"
// #include <ArduinoJson.h>
// #include <SD.h>
// #include "M5StackChanFace.h"

// #define _3D_MODEL
// #ifdef _3D_MODEL
// #include "IcosahedronView.h"
// #include "OctagonRingView.h"
// IcosahedronView icosahedron;
// OctagonRingView octagon;
// #endif

// // 関数プロトタイプ宣言
// void playNoteFromFaceID(int faceID);
// void addFace(int faceID, float x, float y, float z, int led1, int led2, int led3);
// int getNearestFace(float x, float y, float z);
// void lightFaceUpdate();
// void processDetectionState();
// void processCalibrationState();
// void processLedControlState();
// void runIlluminationTest();

// bool isIlluminationTest = false;
// // TaskHandle_t ledTaskHandle = NULL;
// bool ledTaskSuspended = false;


// // スピーカー用のピンと音階
// #define SPEAKER_PIN 25  // M5Stack CoreS3のスピーカー
// const int notes[] = {262, 294, 330, 349, 392, 440, 494, 523}; // ドレミファソラシド

// M5StackChanFace face(0, ACTIONBAR_HEIGHT + (SCREEN_HEIGHT - ACTIONBAR_HEIGHT - TOOLBAR_HEIGHT)/2, SCREEN_WIDTH/2, (SCREEN_HEIGHT - ACTIONBAR_HEIGHT - TOOLBAR_HEIGHT)/2);

// ActionBar actionBar;
// TextView mainTextView;
// TextView subTextView;
// Toolbar toolbar;
// Dialog dialog;

// // センサーしきい値
// #define STABLE_THRESHOLD 0.2 
// #define STABLE_DURATION 1000 * 4
// #define TIMEOUT_DURATION 120000
// bool isViewUpdate = false;

// // LED設定
// // #define LED_PIN 1 // s3 port.a
// #define LED_PIN 8 // din base port.b
// // #define LED_PIN 18 // din base port.c
// #define NUM_FACES 8  // 最大面数
// #define LED_ADDRESS_OFFSET 1  // LEDアドレスのオフセット（0番は未使用）
// #define NUM_LEDS (8 * 2) + LED_ADDRESS_OFFSET  // LEDテープ全体のLED数 アドレス0番は未使用 8面 * 2 + 1
// // LEDリスト
// CRGB leds[NUM_LEDS];

// uint8_t brightness = 255;  
// bool ledState = false;

// // IMUデータ
// float accX, accY, accZ;

// // 前回値
// float prevAccX, prevAccY, prevAccZ;

// // LED FIFOキュー
// enum LEDCommandType {
//   LED_CMD_ON,
//   LED_CMD_OFF,
//   LED_CMD_SET_COLOR,
//   // LED_CMD_FADE, など必要に応じて追加可能
// };

// // LEDコマンド構造体
// struct LEDCommand {
//   LEDCommandType type;
//   int ledIndex;         // 操作対象のLEDインデックス（または面番号など）
//   CRGB color;           // 色変更の場合の色
//   uint8_t brightness;   // 明るさ（必要なら）
//   uint32_t duration;    // 遅延やフェード時間など（必要なら）
// };

// // キューとタスクハンドルのグローバル変数
// QueueHandle_t ledCommandQueue;
// TaskHandle_t ledTaskHandle = NULL;

// // LEDタスク：キューからコマンドを受信して処理する
// void ledTask(void* parameter) {
//   LEDCommand cmd;
//   while (true) {
//     // キューからコマンドを待機（portMAX_DELAYで無期限待機）
//     if (xQueueReceive(ledCommandQueue, &cmd, portMAX_DELAY) == pdPASS) {
//       switch (cmd.type) {
//         case LED_CMD_ON:
//           leds[cmd.ledIndex] = cmd.color;
//           break;
//         case LED_CMD_OFF:
//           leds[cmd.ledIndex] = CRGB::Black;
//           break;
//         case LED_CMD_SET_COLOR:
//           leds[cmd.ledIndex] = cmd.color;
//           break;
//         // 他のコマンドもここで処理
//       }
//       FastLED.show();
//       // もし、durationが設定されていれば、指定時間待機して次のコマンドへ
//       if (cmd.duration > 0) {
//         vTaskDelay(cmd.duration / portTICK_PERIOD_MS);
//       }
//     }
//   }
// }

// // LEDパターン関数の型定義
// typedef void (*LEDPatternFunction)();

// // パターン1: Sequential（既存）
// // 面1～8まで、1秒ごとに順次追加して点灯
// void ledPatternSequential() {
//   for (int i = 0; i < NUM_FACES; i++) {
//     for (int j = 0; j < NUM_FACES; j++) {
//       int idx1 = LED_ADDRESS_OFFSET + (j * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (j * 2) + 1;
//       if (j <= i) {
//         leds[idx1] = CRGB::White;
//         leds[idx2] = CRGB::White;
//       } else {
//         leds[idx1] = CRGB::Black;
//         leds[idx2] = CRGB::Black;
//       }
//     }
//     FastLED.show();
//     for (int k = 0; k < 10; k++) {
//       vTaskDelay(100 / portTICK_PERIOD_MS);
//     }
//   }
// }

// // パターン2: On/Off（既存）
// // 8面全点灯／消灯を5秒間繰り返す
// void ledPatternOnOff() {
//   bool state = true;
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       if (state) {
//         leds[idx1] = CRGB::White;
//         leds[idx2] = CRGB::White;
//       } else {
//         leds[idx1] = CRGB::Black;
//         leds[idx2] = CRGB::Black;
//       }
//     }
//     FastLED.show();
//     state = !state;
//     for (int k = 0; k < 10; k++) {
//       vTaskDelay(100 / portTICK_PERIOD_MS);
//     }
//   }
// }

// // パターン3: Odd/Even（既存）
// // 偶数面／奇数面を交互に1秒ごとに点灯（5秒間）
// void ledPatternOddEven() {
//   bool toggle = false;
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     toggle = !toggle;
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       // 0-indexedの場合、偶数インデックスと奇数インデックスで分ける
//       if (toggle) {
//         if (i % 2 == 0) {
//           leds[idx1] = CRGB::White;
//           leds[idx2] = CRGB::White;
//         } else {
//           leds[idx1] = CRGB::Black;
//           leds[idx2] = CRGB::Black;
//         }
//       } else {
//         if (i % 2 == 1) {
//           leds[idx1] = CRGB::White;
//           leds[idx2] = CRGB::White;
//         } else {
//           leds[idx1] = CRGB::Black;
//           leds[idx2] = CRGB::Black;
//         }
//       }
//     }
//     FastLED.show();
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//   }
// }

// // パターン4: Random（既存）
// // 各面をランダムな色に500ms間隔で点灯（5秒間）
// void ledPatternRandom() {
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       CRGB randColor = CRGB(random(256), random(256), random(256));
//       leds[idx1] = randColor;
//       leds[idx2] = randColor;
//     }
//     FastLED.show();
//     vTaskDelay(500 / portTICK_PERIOD_MS);
//   }
// }

// // パターン5: Wave（既存）
// // 各面で波状のフェードイン・フェードアウト（5秒間）
// void ledPatternWave() {
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       float phase = (millis() / 100.0) + (i * M_PI / 4);
//       uint8_t brightnessVal = (uint8_t)(((sin(phase) + 1.0) / 2.0) * 255);
//       CRGB baseColor = CRGB::Blue;
//       CRGB modulatedColor = baseColor;
//       modulatedColor.nscale8_video(brightnessVal);
//       leds[idx1] = modulatedColor;
//       leds[idx2] = modulatedColor;
//     }
//     FastLED.show();
//     vTaskDelay(50 / portTICK_PERIOD_MS);
//   }
// }

// // パターン6: Rainbow
// // 各面に虹色のグラデーションを流す（5秒間）
// void ledPatternRainbow() {
//   uint8_t hue = 0;
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       // 各面に対して hue にオフセットを加える
//       CRGB color = CHSV(hue + i * 32, 255, 255);
//       leds[idx1] = color;
//       leds[idx2] = color;
//     }
//     FastLED.show();
//     hue++;  // hue を徐々に増加させる
//     vTaskDelay(50 / portTICK_PERIOD_MS);
//   }
// }

// // パターン7: Strobe
// // 全面を高速に白／消灯させるストロボ効果（5秒間）
// void ledPatternStrobe() {
//   unsigned long startTime = millis();
//   bool state = false;
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       if (state) {
//         leds[idx1] = CRGB::White;
//         leds[idx2] = CRGB::White;
//       } else {
//         leds[idx1] = CRGB::Black;
//         leds[idx2] = CRGB::Black;
//       }
//     }
//     FastLED.show();
//     state = !state;
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//   }
// }

// // パターン8: Chase
// // 1面ずつ順次点灯するチェイス効果（各面300ms、5秒間）
// void ledPatternChase() {
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       // まず全面を消灯
//       for (int j = 0; j < NUM_FACES; j++) {
//         int idx1 = LED_ADDRESS_OFFSET + (j * 2);
//         int idx2 = LED_ADDRESS_OFFSET + (j * 2) + 1;
//         leds[idx1] = CRGB::Black;
//         leds[idx2] = CRGB::Black;
//       }
//       // face i を点灯
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       leds[idx1] = CRGB::White;
//       leds[idx2] = CRGB::White;
//       FastLED.show();
//       vTaskDelay(300 / portTICK_PERIOD_MS);
//     }
//   }
// }

// // パターン9: Pulse
// // 全面の明るさを上下させるパルス効果（5秒間）
// void ledPatternPulse() {
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     // 明るくするフェーズ
//     for (uint8_t b = 0; b < 255; b += 5) {
//       for (int i = 0; i < NUM_FACES; i++) {
//         int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//         int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//         leds[idx1] = CRGB::White;
//         leds[idx2] = CRGB::White;
//         leds[idx1].nscale8_video(b);
//         leds[idx2].nscale8_video(b);
//       }
//       FastLED.show();
//       vTaskDelay(30 / portTICK_PERIOD_MS);
//     }
//     // 暗くするフェーズ
//     for (int b = 255; b > 0; b -= 5) {
//       for (int i = 0; i < NUM_FACES; i++) {
//         int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//         int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//         leds[idx1] = CRGB::White;
//         leds[idx2] = CRGB::White;
//         leds[idx1].nscale8_video(b);
//         leds[idx2].nscale8_video(b);
//       }
//       FastLED.show();
//       vTaskDelay(30 / portTICK_PERIOD_MS);
//     }
//   }
// }

// void ledPatternTwinkle() {
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       // 50%の確率でツインクル
//       if (random(100) < 50) {
//         uint8_t bright = random(50, 255);
//         CRGB color = CRGB::White;
//         color.nscale8_video(bright);
//         leds[idx1] = color;
//         leds[idx2] = color;
//       } else {
//         leds[idx1] = CRGB::Black;
//         leds[idx2] = CRGB::Black;
//       }
//     }
//     FastLED.show();
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//   }
// }

// void ledPatternFireFlicker() {
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = 0; i < NUM_FACES; i++) {
//       int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//       int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//       // flicker 値で明るさをランダムに決定
//       uint8_t flicker = random(100, 255);
//       // 炎っぽさを出すため、赤を主体に、緑は flicker の 0～値の一部、青はゼロ
//       CRGB color = CRGB(flicker, random(0, flicker / 2), 0);
//       leds[idx1] = color;
//       leds[idx2] = color;
//     }
//     FastLED.show();
//     vTaskDelay(50 / portTICK_PERIOD_MS);
//   }
// }

// void ledPatternComet() {
//   // まず全LEDを消灯
//   for (int i = 0; i < NUM_LEDS; i++) {
//     leds[i] = CRGB::Black;
//   }
//   FastLED.show();
//   unsigned long startTime = millis();
//   int cometPos = 0;
//   while (millis() - startTime < 5000) {
//     // 各ループごとに全LEDを少しフェードさせる
//     for (int i = 0; i < NUM_LEDS; i++) {
//       leds[i].nscale8_video(200); // 約20%程度の減衰（調整可能）
//     }
//     // 現在のコメット位置の面を白色で点灯
//     int idx1 = LED_ADDRESS_OFFSET + (cometPos * 2);
//     int idx2 = LED_ADDRESS_OFFSET + (cometPos * 2) + 1;
//     leds[idx1] = CRGB::White;
//     leds[idx2] = CRGB::White;
    
//     FastLED.show();
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//     cometPos = (cometPos + 1) % NUM_FACES;
//   }
// }

// // パターン10: Individual Random
// // 各 LED を独立にランダムな色で点灯／消灯させる（約5秒間）
// void ledPatternIndividualRandom() {
//   unsigned long startTime = millis();
//   while (millis() - startTime < 5000) {
//     for (int i = LED_ADDRESS_OFFSET; i < NUM_LEDS; i++) {
//       // 50%の確率でランダムな色にする、そうでなければ消灯
//       if (random(100) < 50) {
//         leds[i] = CRGB(random(256), random(256), random(256));
//       } else {
//         leds[i] = CRGB::Black;
//       }
//     }
//     FastLED.show();
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//   }
// }



// LEDPatternFunction ledPatterns[] = {
//   ledPatternSequential,
//   ledPatternOnOff,
//   ledPatternOddEven,
//   ledPatternRandom,
//   ledPatternWave,
//   ledPatternRainbow,
//   ledPatternStrobe,
//   ledPatternChase,
//   ledPatternPulse,
//   ledPatternTwinkle,
//   ledPatternFireFlicker,
//   ledPatternComet,
//   ledPatternIndividualRandom
// };

// const int numLEDPatterns = sizeof(ledPatterns) / sizeof(ledPatterns[0]);
// int currentLEDPatternIndex = 0;

// const char* ledPatternNames[] = {
//   "Sequential",
//   "On/Off",
//   "Odd/Even",
//   "Random",
//   "Wave",
//   "Rainbow",
//   "Strobe",
//   "Chase",
//   "Pulse",
//   "Twinkle",
//   "FireFlicker",
//   "Comet",
//   "Individual Random"
// };


// // Faceデータ
// struct FaceData {
//     int id;               // 面のID
//     float x, y, z;        // センサー値（重力加速度）
//     int ledAddress[NUM_LEDS];    // 面に対応するLEDテープのアドレス（最大3つ）
//     int numLEDs;          // 使用するLEDの数
//     int ledBrightness;    // LEDの明るさ (0~255)
//     int ledColor;         // LEDの色（RGB値）
//     int ledState;         // LEDの状態（ON=1, OFF=0）
//     int ledPattern;       // LEDの点灯パターン（0=固定, 1=点滅, 2=フェード）
//     int ledFadeSpeed;     // フェード/点滅速度
//     unsigned long ledUpdateTime; // LEDの最終更新時刻（ミリ秒単位）
//     bool isActive;        // この面がアクティブか（デバッグや非アクティブ化用）
// };

// FaceData faceList[NUM_FACES];  
// int calibratedFaces = 0;
// int beforeDetectedFace = -1; // 前回検出した面

// // システム状態
// enum State { STATE_NONE, STATE_DETECTION, STATE_CALIBRATION, STATE_LED_CONTROL };
// State currentState = STATE_NONE;

// // Detectionのサブステート
// enum State_detection { 
//     STATE_DETECTION_INIT, 
//     STATE_DETECTION_DETECT_FACE
// };
// State_detection detectState = STATE_DETECTION_INIT;

// // キャリブレーションのサブステート
// enum State_calibration { 
//     STATE_CALIBRATION_INIT, 
//     STATE_CALIBRATION_WAIT_STABLE, 
//     STATE_CALIBRATION_DETECT_FACE, 
//     STATE_CALIBRATION_CONFIRM_NEW_FACE,
//     STATE_CALIBRATION_COMPLETE
// };
// State_calibration calibrationState = STATE_CALIBRATION_INIT;

// // LED制御のサブステート
// enum State_led_control { 
//     STATE_LED_CONTROL_INIT, 
//     STATE_LED_CONTROL_PAUSED,  // 再生停止中
//     STATE_LED_CONTROL_PLAYING, // 再生中
//     STATE_LED_CONTROL_STOP,
// };
// State_led_control ledControlState = STATE_LED_CONTROL_INIT;

// #define LED_ADDRESS_OFFSET 1  // 例: 配列の1番目からLEDが始まる場合

// void lightFaceUpdate() {
//     for (int i = 0; i < NUM_FACES; i++) {
//         int ledIndex1 = LED_ADDRESS_OFFSET + (i * 2);
//         int ledIndex2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//         if (faceList[i].ledState == 1) {
//             CRGB adjustedColor = faceList[i].ledColor;
//             adjustedColor.nscale8_video(faceList[i].ledBrightness);
//             leds[ledIndex1] = adjustedColor;
//             leds[ledIndex2] = adjustedColor;
//             // leds[ledIndex1] = faceList[i].ledColor;
//             // leds[ledIndex2] = faceList[i].ledColor;
//         } else {
//             leds[ledIndex1] = CRGB::Black;
//             leds[ledIndex2] = CRGB::Black;
//         }
//     }
//     FastLED.show();
// }

// // 音を鳴らす関数
// void playNoteFromFaceID(int faceID) {
//     if (faceID < 0 || faceID >= 8) {
//         Serial.println("Invalid faceID for note");
//         return;
//     }

//     int frequency = notes[faceID]; 
//     Serial.println("Playing note for faceID: " + String(faceID) + " Frequency: " + String(frequency));
//     M5.Speaker.setVolume(150);
//     M5.Speaker.tone(frequency, 200); // 200ms 再生
// }

// // 面データを追加する関数
// void addFace(int faceID, float x, float y, float z, int led1, int led2, int led3) {
//     if (calibratedFaces >= NUM_FACES) return;

//     faceList[calibratedFaces].id = faceID;
//     faceList[calibratedFaces].x = x;
//     faceList[calibratedFaces].y = y;
//     faceList[calibratedFaces].z = z;
//     faceList[calibratedFaces].ledAddress[0] = led1;
//     faceList[calibratedFaces].ledAddress[1] = led2;
//     faceList[calibratedFaces].ledAddress[2] = led3;
//     faceList[calibratedFaces].numLEDs = 2;  // LED 3つ使用
//     faceList[calibratedFaces].ledBrightness = 255;
//     faceList[calibratedFaces].ledColor = CRGB::White;
//     faceList[calibratedFaces].ledState = 0;
//     faceList[calibratedFaces].ledPattern = 0;
//     faceList[calibratedFaces].ledFadeSpeed = 0;
//     faceList[calibratedFaces].ledUpdateTime = 0;
//     faceList[calibratedFaces].isActive = true;
    
//     calibratedFaces++;
// }

// // 内積を利用して最も近い面を判定
// int getNearestFace(float x, float y, float z) {
//     int nearestFace = -1;
//     float maxSimilarity = -1;  // 内積の最大値（-1から1の範囲）

//     for (int i = 0; i < calibratedFaces; i++) {
//         // 正規化された法線ベクトル
//         float Nx = faceList[i].x;
//         float Ny = faceList[i].y;
//         float Nz = faceList[i].z;

//         // 内積を計算（cosθに相当）
//         float dotProduct = x * Nx + y * Ny + z * Nz;

//         // 内積が最大（最も類似した面）を採用
//         if (dotProduct > maxSimilarity) {
//             maxSimilarity = dotProduct;
//             nearestFace = faceList[i].id;
//         }
//     }

//     // cosθ（dotProduct）がしきい値以上の場合のみ採用
//     if (maxSimilarity > 0.95) {  // 0.95は誤差の許容範囲
//         return nearestFace;
//     } else {
//         return -1;
//     }
// }

// // 面検出の処理
// void processDetectionState() {
//     float mag, normX, normY, normZ;
//     int detectedFace = -1;  // 事前に初期化

//     switch (detectState) {
//         case STATE_DETECTION_INIT:
//             mainTextView.setText("Detection mode initialized");
//             detectState = STATE_DETECTION_DETECT_FACE;
//             // toolBarの更新
//             toolbar.setButtonLabel(BTN_A, "");
//             toolbar.setButtonLabel(BTN_B, "");
//             toolbar.setButtonLabel(BTN_C, "");
//             break;

//         case STATE_DETECTION_DETECT_FACE:
//             M5.Imu.getAccel(&accX, &accY, &accZ);
//             subTextView.setText("Detecting face... \nx=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ));

//             mag = sqrt(accX * accX + accY * accY + accZ * accZ);
//             normX = accX / mag;
//             normY = accY / mag;
//             normZ = accZ / mag;
//             detectedFace = getNearestFace(normX, normY, normZ);

//             // 全ての面のLED状態をリセットする
//             for (int i = 0; i < calibratedFaces; i++) {
//                 faceList[i].ledState = 0;
//             }

//             // faceListに登録されている場合
//             if (detectedFace != -1) {
//                 mainTextView.setText("Detected face: " + String(detectedFace) + "\nledAddress: " + faceList[detectedFace].ledAddress[0] + ", " + faceList[detectedFace].ledAddress[1] + "\nledState: " + faceList[detectedFace].ledState);
//                 // ハイライト対象の面を設定
//                 octagon.setHighlightedFace(detectedFace);
//                 faceList[detectedFace].ledState = 1;
//             }
//             else {
//                 mainTextView.setText("No face detected");
//                 // 検出できなかった場合はハイライト解除
//                 octagon.setHighlightedFace(-1);
//             }
//             octagon.draw();
            
//             lightFaceUpdate();
//             delay(50);
//             break;

//     }
// }

// // キャリブレーションの処理
// void processCalibrationState() {
//     float mag, normX, normY, normZ;
//     int detectedFace = -1;  // 事前に初期化

//     switch (calibrationState) {
//         case STATE_CALIBRATION_INIT:
//             mainTextView.setText("Calibration mode initialized");
//             calibrationState = STATE_CALIBRATION_WAIT_STABLE;
            
//             // 前回値のリセット
//             prevAccX = 0;
//             prevAccY = 0;
//             prevAccZ = 0;

//             // toolBarの更新
//             toolbar.setButtonLabel(BTN_A, "RESET");
//             toolbar.setButtonLabel(BTN_B, "SAVE");
//             toolbar.setButtonLabel(BTN_C, "LOAD");
//             toolbar.draw();
//             break;

//         case STATE_CALIBRATION_WAIT_STABLE:
//             mainTextView.setText("Waiting for calibration...\n" + String(calibratedFaces) + " faces calibrated");
            
//             M5.Imu.getAccel(&accX, &accY, &accZ);
//             static unsigned long stableStartTime = millis();
//             subTextView.setText("x=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ)
//                 + "\n" + String(millis() - stableStartTime) + "ms");

//             if (abs(accX - prevAccX) < STABLE_THRESHOLD &&
//                 abs(accY - prevAccY) < STABLE_THRESHOLD &&
//                 abs(accZ - prevAccZ) < STABLE_THRESHOLD) {
//                 if (millis() - stableStartTime > STABLE_DURATION) {
//                     calibrationState = STATE_CALIBRATION_DETECT_FACE;
//                 }
//             } else {
//                 stableStartTime = millis();
//             }

//             prevAccX = accX;
//             prevAccY = accY;
//             prevAccZ = accZ;
//             break;

//         case STATE_CALIBRATION_DETECT_FACE:
//             mainTextView.setText("Detected face\n" + String(calibratedFaces) + " faces calibrated");
//             M5.Imu.getAccel(&accX, &accY, &accZ);
//             subTextView.setText("x=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ));

//             mag = sqrt(accX * accX + accY * accY + accZ * accZ);
//             normX = accX / mag;
//             normY = accY / mag;
//             normZ = accZ / mag;
//             detectedFace = getNearestFace(normX, normY, normZ);

//             if (calibratedFaces >=NUM_FACES) {
//                 String message = "Face list is full. Please save or reset.";
//                 dialog.showDialog("Info", message, DIALOG_OK);
//                 DialogResult dialogResult = DIALOG_NONE;
//                 while (dialogResult == DIALOG_NONE) {
//                     dialogResult = dialog.getResult();
//                     delay(100);
//                 }
//                 if (dialogResult == DIALOG_OK_PRESSED) {
//                     calibrationState = STATE_CALIBRATION_WAIT_STABLE;
//                 }
//             } else if (detectedFace != -1) {
//                 String message = "Detected existing face: " + String(detectedFace) + "\n detect value: " + String(normX) + ", " + String(normY) + ", " + String(normZ); 
//                 dialog.showDialog("Info", message, DIALOG_OK);
//                 DialogResult dialogResult = DIALOG_NONE;
//                 while (dialogResult == DIALOG_NONE) {
//                     dialogResult = dialog.getResult();
//                     delay(100);
//                 }
//                 if (dialogResult == DIALOG_OK_PRESSED) {
//                     calibrationState = STATE_CALIBRATION_WAIT_STABLE;
//                 }                
//             } else {
//                 String message = "New face detected. Add to list?\n detect value: " + String(normX) + ", " + String(normY) + ", " + String(normZ);
//                 dialog.showDialog("Confirm", message, DIALOG_OK_CANCEL);
//                 DialogResult dialogResult = DIALOG_NONE;
//                 while (dialogResult == DIALOG_NONE) {
//                     dialogResult = dialog.getResult();
//                     delay(100);
//                 }
//                 if (dialogResult == DIALOG_OK_PRESSED) {
//                     Serial.println("DIALOG_OK_PRESSED");

//                     // LEDアドレスの設定
//                     // faceList[calibratedFaces] = {calibratedFaces + 1, normX, normY, normZ};
//                     addFace(calibratedFaces, normX, normY, normZ, calibratedFaces, calibratedFaces + 1, calibratedFaces + 2);
//                     calibrationState = STATE_CALIBRATION_WAIT_STABLE;
//                 } else {
//                     Serial.println("DIALOG_CANCEL_PRESSED");
//                     calibrationState = STATE_CALIBRATION_WAIT_STABLE;
//                 }
//             }

//             // 前回値のリセット
//             prevAccX = 0;
//             prevAccY = 0;
//             prevAccZ = 0;

//             isViewUpdate = true;
//             break;

//         default:
//             break;
//     }
// }

// // 面データのリセット
// void resetFaces() {
//     Serial.println("Reset faces");

//     // メモリ上の面データを初期化
//     calibratedFaces = 0;
//     memset(faceList, 0, sizeof(faceList));

//     // SDカードのデータを削除
//     if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
//         Serial.println("SD card initialization failed!");
//         return;
//     }

//     if (SD.exists("/faces.json")) {
//         SD.remove("/faces.json");
//         Serial.println("Deleted faces.json from SD card");
//     } else {
//         Serial.println("No face data found on SD card");
//     }
// }

// // 面データの保存
// // SDカードに保存する
// void saveFaces() {
//     Serial.println("Save faces");

//     JsonDocument jsonDoc;  // 修正: StaticJsonDocument → JsonDocument
//     JsonArray faceArray = jsonDoc["faces"].to<JsonArray>();  // 修正: createNestedArray() の代替

//     for (int i = 0; i < calibratedFaces; i++) {
//         JsonObject faceObj = faceArray.add<JsonObject>();  // 修正: createNestedObject() の代替
//         faceObj["id"] = faceList[i].id;
//         faceObj["x"] = faceList[i].x;
//         faceObj["y"] = faceList[i].y;
//         faceObj["z"] = faceList[i].z;
//         faceObj["led1"] = faceList[i].ledAddress[0];
//         faceObj["led2"] = faceList[i].ledAddress[1];
//         faceObj["led3"] = faceList[i].ledAddress[2];
//         faceObj["numLEDs"] = faceList[i].numLEDs;
//         faceObj["brightness"] = faceList[i].ledBrightness;
//         faceObj["color"] = faceList[i].ledColor;
//         faceObj["state"] = faceList[i].ledState;
//         faceObj["pattern"] = faceList[i].ledPattern;
//         faceObj["fadeSpeed"] = faceList[i].ledFadeSpeed;
//         faceObj["updateTime"] = faceList[i].ledUpdateTime;
//         faceObj["isActive"] = faceList[i].isActive;
    
//     }

//     if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
//         Serial.println("SD card initialization failed!");
//         return;
//     }

//     if (!SD.exists("/faces.json")) {
//         Serial.println("File not found, creating new file");
//     }

//     File file = SD.open("/faces.json", FILE_WRITE);
//     if (!file) {
//         Serial.println("Failed to open file for writing");
//         return;
//     }

//     if (serializeJson(jsonDoc, file) == 0) {
//         Serial.println("Failed to write JSON");
//     }
//     file.close();
// }

// // 面データの読み込み
// void loadFaces() {
//     Serial.println("Load faces");

//     if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
//         Serial.println("SD card initialization failed!");
//         return;
//     }

//     File file = SD.open("/faces.json", FILE_READ);
//     if (!file) {
//         Serial.println("Failed to open file for reading");
//         return;
//     }

//     JsonDocument jsonDoc;
//     DeserializationError error = deserializeJson(jsonDoc, file);
//     if (error) {
//         Serial.println("Failed to parse JSON");
//         return;
//     }

//     JsonArray faceArray = jsonDoc["faces"].as<JsonArray>();
//     calibratedFaces = 0;
//     for (JsonObject faceObj : faceArray) {
//         if (calibratedFaces >= NUM_FACES) break;
//         faceList[calibratedFaces].id = faceObj["id"];
//         faceList[calibratedFaces].x = faceObj["x"];
//         faceList[calibratedFaces].y = faceObj["y"];
//         faceList[calibratedFaces].z = faceObj["z"];
//         faceList[calibratedFaces].ledAddress[0] = faceObj["led1"];
//         faceList[calibratedFaces].ledAddress[1] = faceObj["led2"];
//         // faceList[calibratedFaces].ledAddress[2] = faceObj["led3"];
//         faceList[calibratedFaces].numLEDs = faceObj["numLEDs"];
//         faceList[calibratedFaces].ledBrightness = faceObj["brightness"];
//         faceList[calibratedFaces].ledColor = faceObj["color"];
//         faceList[calibratedFaces].ledState = faceObj["state"];
//         faceList[calibratedFaces].ledPattern = faceObj["pattern"];
//         faceList[calibratedFaces].ledFadeSpeed = faceObj["fadeSpeed"];
//         faceList[calibratedFaces].ledUpdateTime = faceObj["updateTime"];
//         faceList[calibratedFaces].isActive = faceObj["isActive"];

//         // calibratedFaces=0のときに、leds[1]とleds[2]に色を設定, 1のときに、leds[3]とleds[4]に色を設定
//         leds[1 + (calibratedFaces * 2)] = faceList[calibratedFaces].ledColor;
//         leds[1 + (calibratedFaces * 2 + 1)] = faceList[calibratedFaces].ledColor;
//         calibratedFaces++;
//     }
//     file.close();
// }

// // 面の検出→その面を点灯させてみる→実際の面と一致していない場合にledAddressを修正する必要がある
// // そのためのキャリブレーション
// void processLEDControlState() {
//     switch (ledControlState) {
//         case STATE_LED_CONTROL_INIT:
//             toolbar.setButtonLabel(BTN_A, "play");
//             toolbar.setButtonLabel(BTN_B, "prev");
//             toolbar.setButtonLabel(BTN_C, "next");
//             // 初期状態は再生停止中とする
//             ledControlState = STATE_LED_CONTROL_PAUSED;
//             mainTextView.setText(String("Pattern: ") + ledPatternNames[currentLEDPatternIndex]);
//             isViewUpdate = true;
//             break;

//         case STATE_LED_CONTROL_PAUSED:
//             // パターン切り替え（prev, next）はどちらでも受け付ける
//             if (toolbar.getPressedButton(BTN_B)) { // prevボタン
//                 currentLEDPatternIndex = (currentLEDPatternIndex - 1 + numLEDPatterns) % numLEDPatterns;
//                 mainTextView.setText(String("Pattern: ") + ledPatternNames[currentLEDPatternIndex]);
//                 delay(300);
//             }
//             if (toolbar.getPressedButton(BTN_C)) { // nextボタン
//                 currentLEDPatternIndex = (currentLEDPatternIndex + 1) % numLEDPatterns;
//                 mainTextView.setText(String("Pattern: ") + ledPatternNames[currentLEDPatternIndex]);
//                 delay(300);
//             }
//             // 再生要求（BTN_A）
//             if (toolbar.getPressedButton(BTN_A)) {
//                 // 現在 PAUSED なら再生開始
//                 if (ledTaskHandle != NULL) {
//                     vTaskResume(ledTaskHandle);
//                     ledControlState = STATE_LED_CONTROL_PLAYING;
//                     mainTextView.setText(String("Playing: ") + ledPatternNames[currentLEDPatternIndex]);
//                     toolbar.setButtonLabel(BTN_A, "pause");
//                     toolbar.draw();
//                 }
//                 delay(300);
//             }
//             break;

//         case STATE_LED_CONTROL_PLAYING:
//             // パターン切り替えも可能ならここでもチェック（必要に応じて）
//             if (toolbar.getPressedButton(BTN_B)) { // prevボタン
//                 currentLEDPatternIndex = (currentLEDPatternIndex - 1 + numLEDPatterns) % numLEDPatterns;
//                 mainTextView.setText(String("Pattern: ") + ledPatternNames[currentLEDPatternIndex]);
//                 delay(300);
//             }
//             if (toolbar.getPressedButton(BTN_C)) { // nextボタン
//                 currentLEDPatternIndex = (currentLEDPatternIndex + 1) % numLEDPatterns;
//                 mainTextView.setText(String("Pattern: ") + ledPatternNames[currentLEDPatternIndex]);
//                 delay(300);
//             }
//             // 停止要求（BTN_A）
//             if (toolbar.getPressedButton(BTN_A)) {
//                 if (ledTaskHandle != NULL) {
//                     vTaskSuspend(ledTaskHandle);
//                     ledControlState = STATE_LED_CONTROL_PAUSED;
//                     mainTextView.setText(String("Paused: ") + ledPatternNames[currentLEDPatternIndex]);
//                     toolbar.setButtonLabel(BTN_A, "play");
//                     toolbar.draw();
//                 }
//                 delay(300);
//             }
//             break;
//         default:
//             break;
//     }
// }

// // メインループの処理
// void processState() {
//     switch (currentState) {
//         case STATE_DETECTION:
//             actionBar.setTitle("Face Detection");
//             processDetectionState();
//             break;
//         case STATE_CALIBRATION:
//             actionBar.setTitle("Face Calibration");
//             processCalibrationState();
//             break;
//         case STATE_LED_CONTROL:
//             actionBar.setTitle("LED Control");
//             processLEDControlState();
//             break;
//         default:
//             break;
//     }

// }

// // ステート変更
// void changeState(State newState) {
//     Serial.println("Change state: " + String(newState));
//     currentState = newState;
//     isViewUpdate = true;
    
// }

// // サンプル：8面を使ったイルミネーションパターン
// void runIlluminationTest() {
//   // パターンを無限ループで実行（終了条件は用途に合わせて変更してください）
//   while(true) {
//     // 1. 面1～8まで、1秒ごとに順次追加して点灯
//     for (int i = 0; i < NUM_FACES; i++) {
//       for (int j = 0; j < NUM_FACES; j++) {
//         int idx1 = LED_ADDRESS_OFFSET + (j * 2);
//         int idx2 = LED_ADDRESS_OFFSET + (j * 2) + 1;
//         if (j <= i) {
//           leds[idx1] = CRGB::White;
//           leds[idx2] = CRGB::White;
//         } else {
//           leds[idx1] = CRGB::Black;
//           leds[idx2] = CRGB::Black;
//         }
//       }
//       FastLED.show();
//       delay(1000);
//     }
    
//     // 2. 8面全点灯後、1秒ごとに点灯／消灯を5秒間繰り返す
//     {
//       bool state = true;
//       unsigned long startTime = millis();
//       while(millis() - startTime < 5000) {
//         for (int i = 0; i < NUM_FACES; i++) {
//           int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//           int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//           if (state) {
//             leds[idx1] = CRGB::White;
//             leds[idx2] = CRGB::White;
//           } else {
//             leds[idx1] = CRGB::Black;
//             leds[idx2] = CRGB::Black;
//           }
//         }
//         FastLED.show();
//         state = !state;
//         delay(1000);
//       }
//     }
    
//     // 3. 偶数面／奇数面を交互に、1秒ごとに点灯（5秒間）
//     {
//       bool toggle = false;  // toggle==false: even faces (番号2,4,6,8) lit, true: odd faces (番号1,3,5,7) lit
//       unsigned long startTime = millis();
//       while(millis() - startTime < 5000) {
//         toggle = !toggle;
//         for (int i = 0; i < NUM_FACES; i++) {
//           int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//           int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//           // 0-indexedの場合、i==0,2,4,6は「1,3,5,7面」、i==1,3,5,7は「2,4,6,8面」
//           if (toggle) {  // 奇数面（実際は0,2,4,6）
//             if (i % 2 == 0) {
//               leds[idx1] = CRGB::White;
//               leds[idx2] = CRGB::White;
//             } else {
//               leds[idx1] = CRGB::Black;
//               leds[idx2] = CRGB::Black;
//             }
//           } else {       // 偶数面（実際は1,3,5,7）
//             if (i % 2 == 1) {
//               leds[idx1] = CRGB::White;
//               leds[idx2] = CRGB::White;
//             } else {
//               leds[idx1] = CRGB::Black;
//               leds[idx2] = CRGB::Black;
//             }
//           }
//         }
//         FastLED.show();
//         delay(1000);
//       }
//     }
    
//     // 4. 各面をランダムに色を変えて点灯（500ms間隔で5秒間）
//     {
//       unsigned long startTime = millis();
//       while(millis() - startTime < 5000) {
//         for (int i = 0; i < NUM_FACES; i++) {
//           int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//           int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//           CRGB randColor = CRGB(random(256), random(256), random(256));
//           leds[idx1] = randColor;
//           leds[idx2] = randColor;
//         }
//         FastLED.show();
//         delay(500);
//       }
//     }
    
//     // 5. ウェーブ：8面で波状のフェードイン・フェードアウトを5秒間（各面に位相オフセット）
//     {
//       unsigned long startTime = millis();
//       while(millis() - startTime < 5000) {
//         for (int i = 0; i < NUM_FACES; i++) {
//           int idx1 = LED_ADDRESS_OFFSET + (i * 2);
//           int idx2 = LED_ADDRESS_OFFSET + (i * 2) + 1;
//           // 各面に対して、時間と面番号によるsin波で輝度を計算
//           float phase = (millis() / 100.0) + (i * PI / 4);
//           uint8_t brightnessVal = (uint8_t)(((sin(phase) + 1.0) / 2.0) * 255);
//           // 例としてベースカラーは青
//           CRGB baseColor = CRGB::Blue;
//           CRGB modulatedColor = baseColor;
//           modulatedColor.nscale8_video(brightnessVal);
//           leds[idx1] = modulatedColor;
//           leds[idx2] = modulatedColor;
//         }
//         FastLED.show();
//         delay(50);
//       }
//     }
    
//     // 6. 1のパターンに戻ってループ
//   }
// }

// void taskIllumination(void *parameter) {
//   // runIlluminationTest() は内部で無限ループしている前提
// //   runIlluminationTest();
// //   // ここには到達しませんが、タスク終了時に削除する場合は以下を呼び出します
//     while (true) {
//         // LEDパターンを実行
//         ledPatterns[currentLEDPatternIndex]();
//         // パターン実行が終わったら、タスクを一時停止しておく
//         // vTaskSuspend(NULL);
//     }
//     vTaskDelete(NULL);
// }


// void setup() {
//     M5.begin();
//     Serial.begin(115200);
    
//     actionBar.begin();
//     actionBar.setTitle("Main Menu");
//     actionBar.setStatus("Ready");
//     actionBar.draw();

//     mainTextView.begin();
//     mainTextView.setPosition(0, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3*2, SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT);
//     mainTextView.setFontSize(2);
//     mainTextView.setColor(WHITE);
//     mainTextView.setBackgroundColor(BLACK);
//     mainTextView.setText("System Ready");
//     mainTextView.draw();

//     int subViewHeight = (SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT)/2;

//     subTextView.begin();
//     subTextView.setPosition(SCREEN_WIDTH/3*2, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3, subViewHeight);
//     subTextView.setFontSize(1);
//     subTextView.setColor(WHITE);
//     subTextView.setBackgroundColor(BLACK);
//     subTextView.setText("");
//     subTextView.draw();

//     // subTextViewの下に表示されるようにする
//     octagon.setViewPosition(SCREEN_WIDTH/3*2, ACTIONBAR_HEIGHT+subViewHeight, SCREEN_WIDTH/3, subViewHeight);

//     toolbar.begin();
//     toolbar.setButtonLabel(BTN_A, "Detect");
//     toolbar.setButtonLabel(BTN_B, "Calib");
//     toolbar.setButtonLabel(BTN_C, "LED");
//     toolbar.draw();

//     prevAccX = 0;
//     prevAccY = 0;
//     prevAccZ = 0;

//     loadFaces();
//     FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
//     FastLED.setBrightness(brightness);
//     // 全てのLEDを消灯
//     for (int i = 0; i < NUM_LEDS; i++) {
//         leds[i] = CRGB::Black;
//     }
//     FastLED.show();


//     // LEDイルミネーションを別タスクで実行（例: Core1で実行）
//     xTaskCreatePinnedToCore(
//         taskIllumination,  // タスク関数
//         "LEDTask",         // タスク名
//         4096,              // スタックサイズ（必要に応じて調整）
//         NULL,              // パラメータ
//         1,                 // 優先度（必要に応じて調整）
//         &ledTaskHandle,    // タスクハンドル（不要ならNULL）
//         1                  // 実行するコア（例：1）
//     );
//     vTaskSuspend(ledTaskHandle);  // 一時停止

//     //octagonを1面分回転
//     octagon.rotate(0.3926991);
//     octagon.draw();
// }

// bool isLEDTest = true;
// bool testLed = false;

// void someFunctionThatSendsLEDCommands() {
//   LEDCommand command;
//   command.type = LED_CMD_ON;
//   command.ledIndex = 1;         // 例としてインデックス1
//   command.color = CRGB::White;  // 白色に点灯
//   command.brightness = 255;     // フル輝度
//   command.duration = 500;       // 500ms 待機（必要なら）
  
//   // キューにコマンド送信
//   if (xQueueSend(ledCommandQueue, &command, portMAX_DELAY) != pdPASS) {
//     Serial.println("Failed to send LED command");
//   }
// }

// void loop() {
//     CRGB randColor = CRGB(random(255), random(255), random(255));
//     face.update();

//     if (actionBar.isBackPressed()) {
//         Serial.println("Back button pressed!");
//         currentState = STATE_NONE;
//         detectState = STATE_DETECTION_INIT;
//         calibrationState = STATE_CALIBRATION_INIT;
//         ledControlState = STATE_LED_CONTROL_INIT;
//         if (ledTaskHandle != NULL) {
//             vTaskSuspend(ledTaskHandle);
//         }
//         // 全てのLEDを消灯
//         for (int i = 0; i < NUM_LEDS; i++) {
//             leds[i] = CRGB::Black;
//         }
//         FastLED.show();

//         isIlluminationTest = false;

//         actionBar.setTitle("Main Menu");
//         actionBar.setStatus("Ready");
//         actionBar.draw();
//         mainTextView.setText("System Ready");
//         subTextView.setText("");

//         toolbar.setButtonLabel(BTN_A, "Detect");
//         toolbar.setButtonLabel(BTN_B, "Calib");
//         toolbar.setButtonLabel(BTN_C, "LED");
//     }

//     switch (currentState) {
//         case STATE_DETECTION:
//             break;
//         case STATE_CALIBRATION:
//             if (toolbar.getPressedButton(BTN_A)) resetFaces();
//             if (toolbar.getPressedButton(BTN_B)) saveFaces();
//             if (toolbar.getPressedButton(BTN_C)) loadFaces();
//             break;
//         case STATE_LED_CONTROL:
//             // if (toolbar.getPressedButton(BTN_A)) {
//             //     // LEDテストの開始／停止
//             //     if (!ledTaskSuspended && ledTaskHandle != NULL) {
//             //         vTaskResume(ledTaskHandle);
//             //         ledTaskSuspended = true;
//             //         ledControlState = STATE_LED_CONTROL_TEST;
//             //     }
//             //     if (ledTaskSuspended && ledTaskHandle != NULL) {
//             //         vTaskSuspend(ledTaskHandle);
//             //         ledTaskSuspended = false;
//             //         ledControlState = STATE_LED_CONTROL_STOP;
//             //     }
//             // }
//             // if (toolbar.getPressedButton(BTN_B)) {
//             //     // LEDパターンを変更
//             // }
//             // if (toolbar.getPressedButton(BTN_C)) {
//             //     // LEDパターンを変更
//             // }
//             break;
//         default:
//             // メインメニュー
//             if (toolbar.getPressedButton(BTN_A)) changeState(STATE_DETECTION);
//             if (toolbar.getPressedButton(BTN_B)) changeState(STATE_CALIBRATION);
//             if (toolbar.getPressedButton(BTN_C)) changeState(STATE_LED_CONTROL);
//             break;
//     }
//     processState();

//     if (isViewUpdate) {
//         mainTextView.draw();
//         actionBar.draw();
//         toolbar.draw();
//         isViewUpdate = false;
//         octagon.draw();
//     }

//     face.draw();
//     delay(200);
// }

#include <Arduino.h>
#include <M5Unified.h>
#include "OctaController.h"

// メインコントローラのインスタンス
OctaController* controller = nullptr;

void setup() {
  // M5.begin()はOctaController内で呼び出されるため、ここでは呼ばない
  
  // コントローラの作成
  controller = new OctaController();
  
  // コントローラの初期化
  controller->setup();
}

void loop() {
  // M5.update()はUIManagerクラス内で呼び出される
  
  // コントローラのメインループを実行
  controller->loop();
}

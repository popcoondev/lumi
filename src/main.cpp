#include <Arduino.h>
#include <M5Unified.h>
#include "Toolbar.h"
#include <FastLED.h>
#include "TextView.h"
#include "ActionBar.h"
#include "Dialog.h"
#include <ArduinoJson.h>
#include <SD.h>

#define _3D_MODEL
#ifdef _3D_MODEL
#include "IcosahedronView.h"
IcosahedronView icosahedron;

#endif

// スピーカー用のピンと音階
#define SPEAKER_PIN 25  // M5Stack CoreS3のスピーカー
const int notes[] = {262, 294, 330, 349, 392, 440, 494, 523}; // ドレミファソラシド


ActionBar actionBar;
TextView mainTextView;
TextView subTextView;
Toolbar toolbar;
Dialog dialog;

// センサーしきい値
#define STABLE_THRESHOLD 0.2 
#define STABLE_DURATION 1000 * 4
#define TIMEOUT_DURATION 120000
bool isViewUpdate = false;

// LED設定
#define LED_PIN 8    
#define NUM_FACES 20  // 最大面数
#define NUM_LEDS 20   // LEDテープ全体のLED数
// LEDリスト
CRGB leds[NUM_LEDS];

uint8_t brightness = 10;  
bool ledState = false;

// IMUデータ
float accX, accY, accZ;

// 前回値
float prevAccX, prevAccY, prevAccZ;

// Faceデータ
struct FaceData {
    int id;               // 面のID
    float x, y, z;        // センサー値（重力加速度）
    int ledAddress[3];    // 面に対応するLEDテープのアドレス（最大3つ）
    int numLEDs;          // 使用するLEDの数
    int ledBrightness;    // LEDの明るさ (0~255)
    int ledColor;         // LEDの色（RGB値）
    int ledState;         // LEDの状態（ON=1, OFF=0）
    int ledPattern;       // LEDの点灯パターン（0=固定, 1=点滅, 2=フェード）
    int ledFadeSpeed;     // フェード/点滅速度
    unsigned long ledUpdateTime; // LEDの最終更新時刻（ミリ秒単位）
    bool isActive;        // この面がアクティブか（デバッグや非アクティブ化用）
};

FaceData faceList[NUM_FACES];  
int calibratedFaces = 0;
int beforeDetectedFace = -1; // 前回検出した面

// システム状態
enum State { STATE_NONE, STATE_DETECTION, STATE_CALIBRATION, STATE_LED_CONTROL };
State currentState = STATE_NONE;

// Detectionのサブステート
enum State_detection { 
    STATE_DETECTION_INIT, 
    STATE_DETECTION_DETECT_FACE
};
State_detection detectState = STATE_DETECTION_INIT;

// キャリブレーションのサブステート
enum State_calibration { 
    STATE_CALIBRATION_INIT, 
    STATE_CALIBRATION_WAIT_STABLE, 
    STATE_CALIBRATION_DETECT_FACE, 
    STATE_CALIBRATION_CONFIRM_NEW_FACE,
    STATE_CALIBRATION_COMPLETE
};
State_calibration calibrationState = STATE_CALIBRATION_INIT;

// LED制御のサブステート
enum State_led_control { 
    STATE_LED_CONTROL_INIT, 
    STATE_LED_CONTROL_DETECT_FACE, 
    STATE_LED_CONTROL_UPDATE_LED,
    STATE_LED_CONTROL_COMPLETE
};
State_led_control ledControlState = STATE_LED_CONTROL_INIT;

// 音を鳴らす関数
void playNoteFromFaceID(int faceID) {
    if (faceID < 0 || faceID >= 8) {
        Serial.println("Invalid faceID for note");
        return;
    }

    int frequency = notes[faceID]; 
    Serial.println("Playing note for faceID: " + String(faceID) + " Frequency: " + String(frequency));
    M5.Speaker.setVolume(150);
    M5.Speaker.tone(frequency, 200); // 200ms 再生
}

// 面データを追加する関数
void addFace(int faceID, float x, float y, float z, int led1, int led2, int led3) {
    if (calibratedFaces >= NUM_FACES) return;

    faceList[calibratedFaces].id = faceID;
    faceList[calibratedFaces].x = x;
    faceList[calibratedFaces].y = y;
    faceList[calibratedFaces].z = z;
    faceList[calibratedFaces].ledAddress[0] = led1;
    faceList[calibratedFaces].ledAddress[1] = led2;
    faceList[calibratedFaces].ledAddress[2] = led3;
    faceList[calibratedFaces].numLEDs = 3;  // LED 3つ使用
    faceList[calibratedFaces].ledBrightness = 255;
    faceList[calibratedFaces].ledColor = CRGB::White;
    faceList[calibratedFaces].ledState = 0;
    faceList[calibratedFaces].ledPattern = 0;
    faceList[calibratedFaces].ledFadeSpeed = 0;
    faceList[calibratedFaces].ledUpdateTime = 0;
    faceList[calibratedFaces].isActive = true;
    
    calibratedFaces++;
}

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

// 面検出の処理
void processDetectionState() {
    float mag, normX, normY, normZ;
    int detectedFace = -1;  // 事前に初期化

    switch (detectState) {
        case STATE_DETECTION_INIT:
            mainTextView.setText("Detection mode initialized");
            detectState = STATE_DETECTION_DETECT_FACE;
            // toolBarの更新
            toolbar.setButtonLabel(BTN_A, "");
            toolbar.setButtonLabel(BTN_B, "");
            toolbar.setButtonLabel(BTN_C, "");
            break;

        case STATE_DETECTION_DETECT_FACE:
            M5.Imu.getAccel(&accX, &accY, &accZ);
            subTextView.setText("Detecting face... \nx=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ));

            mag = sqrt(accX * accX + accY * accY + accZ * accZ);
            normX = accX / mag;
            normY = accY / mag;
            normZ = accZ / mag;
            detectedFace = getNearestFace(normX, normY, normZ);

            // faceListに登録されている場合
            if (detectedFace != -1) {
                mainTextView.setText("Detected face: " + String(detectedFace));
                // playNoteFromFaceID(detectedFace % 8);
                // ハイライト対象の面を設定
                icosahedron.setHighlightedFace(detectedFace);
            }
            else {
                mainTextView.setText("");
                // 検出できなかった場合はハイライト解除
                icosahedron.setHighlightedFace(-1);
            }
            
            delay(100);
            break;

    }
}

// キャリブレーションの処理
void processCalibrationState() {
    float mag, normX, normY, normZ;
    int detectedFace = -1;  // 事前に初期化

    switch (calibrationState) {
        case STATE_CALIBRATION_INIT:
            mainTextView.setText("Calibration mode initialized");
            calibrationState = STATE_CALIBRATION_WAIT_STABLE;
            
            // 前回値のリセット
            prevAccX = 0;
            prevAccY = 0;
            prevAccZ = 0;

            // toolBarの更新
            toolbar.setButtonLabel(BTN_A, "RESET");
            toolbar.setButtonLabel(BTN_B, "SAVE");
            toolbar.setButtonLabel(BTN_C, "LOAD");
            toolbar.draw();
            break;

        case STATE_CALIBRATION_WAIT_STABLE:
            mainTextView.setText("Waiting for calibration...\n" + String(calibratedFaces) + " faces calibrated");
            
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
            mainTextView.setText("Detected face\n" + String(calibratedFaces) + " faces calibrated");
            M5.Imu.getAccel(&accX, &accY, &accZ);
            subTextView.setText("x=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ));

            mag = sqrt(accX * accX + accY * accY + accZ * accZ);
            normX = accX / mag;
            normY = accY / mag;
            normZ = accZ / mag;
            detectedFace = getNearestFace(normX, normY, normZ);

            if (calibratedFaces >=NUM_FACES) {
                String message = "Face list is full. Please save or reset.";
                dialog.showDialog("Info", message, DIALOG_OK);
                DialogResult dialogResult = DIALOG_NONE;
                while (dialogResult == DIALOG_NONE) {
                    dialogResult = dialog.getResult();
                    delay(100);
                }
                if (dialogResult == DIALOG_OK_PRESSED) {
                    calibrationState = STATE_CALIBRATION_WAIT_STABLE;
                }
            } else if (detectedFace != -1) {
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

                    // LEDアドレスの設定
                    // faceList[calibratedFaces] = {calibratedFaces + 1, normX, normY, normZ};
                    addFace(calibratedFaces, normX, normY, normZ, calibratedFaces, calibratedFaces + 1, calibratedFaces + 2);
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

        default:
            break;
    }
}

// 面データのリセット
void resetFaces() {
    Serial.println("Reset faces");

    // メモリ上の面データを初期化
    calibratedFaces = 0;
    memset(faceList, 0, sizeof(faceList));

    // SDカードのデータを削除
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD card initialization failed!");
        return;
    }

    if (SD.exists("/faces.json")) {
        SD.remove("/faces.json");
        Serial.println("Deleted faces.json from SD card");
    } else {
        Serial.println("No face data found on SD card");
    }
}

// 面データの保存
// SDカードに保存する
void saveFaces() {
    Serial.println("Save faces");

    JsonDocument jsonDoc;  // 修正: StaticJsonDocument → JsonDocument
    JsonArray faceArray = jsonDoc["faces"].to<JsonArray>();  // 修正: createNestedArray() の代替

    for (int i = 0; i < calibratedFaces; i++) {
        JsonObject faceObj = faceArray.add<JsonObject>();  // 修正: createNestedObject() の代替
        faceObj["id"] = faceList[i].id;
        faceObj["x"] = faceList[i].x;
        faceObj["y"] = faceList[i].y;
        faceObj["z"] = faceList[i].z;
        faceObj["led1"] = faceList[i].ledAddress[0];
        faceObj["led2"] = faceList[i].ledAddress[1];
        faceObj["led3"] = faceList[i].ledAddress[2];
        faceObj["numLEDs"] = faceList[i].numLEDs;
        faceObj["brightness"] = faceList[i].ledBrightness;
        faceObj["color"] = faceList[i].ledColor;
        faceObj["state"] = faceList[i].ledState;
        faceObj["pattern"] = faceList[i].ledPattern;
        faceObj["fadeSpeed"] = faceList[i].ledFadeSpeed;
        faceObj["updateTime"] = faceList[i].ledUpdateTime;
        faceObj["isActive"] = faceList[i].isActive;
    
    }

    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD card initialization failed!");
        return;
    }

    if (!SD.exists("/faces.json")) {
        Serial.println("File not found, creating new file");
    }

    File file = SD.open("/faces.json", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    if (serializeJson(jsonDoc, file) == 0) {
        Serial.println("Failed to write JSON");
    }
    file.close();
}

// 面データの読み込み
void loadFaces() {
    Serial.println("Load faces");

    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD card initialization failed!");
        return;
    }

    File file = SD.open("/faces.json", FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    JsonDocument jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, file);
    if (error) {
        Serial.println("Failed to parse JSON");
        return;
    }

    JsonArray faceArray = jsonDoc["faces"].as<JsonArray>();
    calibratedFaces = 0;
    for (JsonObject faceObj : faceArray) {
        if (calibratedFaces >= NUM_FACES) break;
        faceList[calibratedFaces].id = faceObj["id"];
        faceList[calibratedFaces].x = faceObj["x"];
        faceList[calibratedFaces].y = faceObj["y"];
        faceList[calibratedFaces].z = faceObj["z"];
        faceList[calibratedFaces].ledAddress[0] = faceObj["led1"];
        faceList[calibratedFaces].ledAddress[1] = faceObj["led2"];
        faceList[calibratedFaces].ledAddress[2] = faceObj["led3"];
        faceList[calibratedFaces].numLEDs = faceObj["numLEDs"];
        faceList[calibratedFaces].ledBrightness = faceObj["brightness"];
        faceList[calibratedFaces].ledColor = faceObj["color"];
        faceList[calibratedFaces].ledState = faceObj["state"];
        faceList[calibratedFaces].ledPattern = faceObj["pattern"];
        faceList[calibratedFaces].ledFadeSpeed = faceObj["fadeSpeed"];
        faceList[calibratedFaces].ledUpdateTime = faceObj["updateTime"];
        faceList[calibratedFaces].isActive = faceObj["isActive"];

        calibratedFaces++;
    }
    file.close();
}

void lightUpFaceAll() {
    for (int i = 0; i < calibratedFaces; i++) {
        if (faceList[i].isActive) {
            for (int j = 0; j < faceList[i].numLEDs; j++) {
                int ledIndex = faceList[i].ledAddress[j];
                if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
                    // LEDの状態に応じて色を決定
                    if (faceList[i].ledState == 1) {
                        leds[ledIndex] = faceList[i].ledColor;
                    } else {
                        leds[ledIndex] = CRGB::Black;
                    }
                }
            }
        }
    }
    FastLED.show();
}

void lightUpFace(int faceID) {
    for (int i = 0; i < calibratedFaces; i++) {
        if (faceList[i].id == faceID && faceList[i].isActive) {
            for (int j = 0; j < faceList[i].numLEDs; j++) {
                int ledIndex = faceList[i].ledAddress[j];
                if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
                    // LEDの状態に応じて色を決定
                    if (faceList[i].ledState == 1) {
                        leds[ledIndex] = faceList[i].ledColor;
                        

                    } else {
                        leds[ledIndex] = CRGB::Black;
                    }
                }
            }
            FastLED.show();
            return;
        }
    }
}

void lightDownFace(int faceID) {
    for (int i = 0; i < calibratedFaces; i++) {
        if (faceList[i].id == faceID && faceList[i].isActive) {
            for (int j = 0; j < faceList[i].numLEDs; j++) {
                int ledIndex = faceList[i].ledAddress[j];
                if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
                    leds[ledIndex] = CRGB::Black;
                }
            }
            FastLED.show();
            return;
        }
    }
}

// 面の検出→その面を点灯させてみる→実際の面と一致していない場合にledAddressを修正する必要がある
// そのためのキャリブレーション
void processLEDControlState() {
    float mag, normX, normY, normZ;
    int detectedFace = -1;  // 事前に初期化
    int faceId = -1;
    switch (ledControlState) {
        case STATE_LED_CONTROL_INIT:
            mainTextView.setText("LED Control mode initialized");
            toolbar.setButtonLabel(BTN_A, "prev");
            toolbar.setButtonLabel(BTN_B, "next");
            toolbar.setButtonLabel(BTN_C, "update");
            isViewUpdate = true;
            ledControlState = STATE_LED_CONTROL_DETECT_FACE;
            break;
        case STATE_LED_CONTROL_DETECT_FACE:
            mainTextView.setText("Detecting face...");
            // 面が検出されたら、その顔に対応するLEDを点灯させる
            M5.Imu.getAccel(&accX, &accY, &accZ);
            subTextView.setText("x=" + String(accX) + "\ny=" + String(accY) + "\nz=" + String(accZ));

            mag = sqrt(accX * accX + accY * accY + accZ * accZ);
            normX = accX / mag;
            normY = accY / mag;
            normZ = accZ / mag;
            detectedFace = getNearestFace(normX, normY, normZ);

            // faceListに登録されている場合
            if (detectedFace != -1) {
                lightUpFace(detectedFace);
                beforeDetectedFace = detectedFace;
                ledControlState = STATE_LED_CONTROL_UPDATE_LED;
            }
            
            delay(100);
            break;
        
        case STATE_LED_CONTROL_UPDATE_LED:
            faceId = faceList[beforeDetectedFace].id;
            mainTextView.setText("ID:" + String(faceId) + "\nLED Address: " + String(faceList[beforeDetectedFace].ledAddress[0]) + ", " + String(faceList[beforeDetectedFace].ledAddress[1]) + ", " + String(faceList[beforeDetectedFace].ledAddress[2]));
            // toolbarでprev, next, updateを待機
            // LEDアドレスを3つずつずらしてみる（NUM_LED以上になったら0に戻る、-1以下になったらNUM_LED-1に戻る）
            if (toolbar.getPressedButton(BTN_A)) {
                lightDownFace(beforeDetectedFace);
                if (faceList[beforeDetectedFace].ledAddress[0] == 0 ) {
                    faceList[beforeDetectedFace].ledAddress[0] = NUM_LEDS - 3;
                    faceList[beforeDetectedFace].ledAddress[1] = NUM_LEDS - 2;
                    faceList[beforeDetectedFace].ledAddress[2] = NUM_LEDS - 1;
                } else {
                    faceList[beforeDetectedFace].ledAddress[0] -= 3;
                    faceList[beforeDetectedFace].ledAddress[1] -= 3;
                    faceList[beforeDetectedFace].ledAddress[2] -= 3;
                }
                lightUpFace(beforeDetectedFace);
                mainTextView.setText("ID:" + String(faceId) + "\nLED Address: " + String(faceList[beforeDetectedFace].ledAddress[0]) + ", " + String(faceList[beforeDetectedFace].ledAddress[1]) + ", " + String(faceList[beforeDetectedFace].ledAddress[2]));
            }
            if (toolbar.getPressedButton(BTN_B)) {
                lightDownFace(beforeDetectedFace);
                if (faceList[beforeDetectedFace].ledAddress[2] == NUM_LEDS - 1) {
                    faceList[beforeDetectedFace].ledAddress[0] = 0;
                    faceList[beforeDetectedFace].ledAddress[1] = 1;
                    faceList[beforeDetectedFace].ledAddress[2] = 2;
                } else {
                    faceList[beforeDetectedFace].ledAddress[0] += 3;
                    faceList[beforeDetectedFace].ledAddress[1] += 3;
                    faceList[beforeDetectedFace].ledAddress[2] += 3;
                }
                lightUpFace(beforeDetectedFace);
                mainTextView.setText("ID:" + String(faceId) + "\nLED Address: " + String(faceList[beforeDetectedFace].ledAddress[0]) + ", " + String(faceList[beforeDetectedFace].ledAddress[1]) + ", " + String(faceList[beforeDetectedFace].ledAddress[2]));
            }
            if (toolbar.getPressedButton(BTN_C)) {
                // faceListの更新
                saveFaces();
                // 対象の面を点滅させる
                for (int i = 0; i < 2; i++) {
                    lightUpFace(beforeDetectedFace);
                    delay(100);
                    lightDownFace(beforeDetectedFace);
                    delay(100);
                }

                ledControlState = STATE_LED_CONTROL_DETECT_FACE;
                beforeDetectedFace = -1;
            }
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
            processDetectionState();
            break;
        case STATE_CALIBRATION:
            actionBar.setTitle("Face Calibration");
            processCalibrationState();
            break;
        case STATE_LED_CONTROL:
            actionBar.setTitle("LED Control");
            processLEDControlState();
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

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
    
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

    int subViewHeight = (SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT)/2;

    subTextView.begin();
    subTextView.setPosition(SCREEN_WIDTH/3*2, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3, subViewHeight);
    subTextView.setFontSize(1);
    subTextView.setColor(WHITE);
    subTextView.setBackgroundColor(BLACK);
    subTextView.setText("");
    subTextView.draw();

    // subTextViewの下に表示されるようにする
    icosahedron.setViewPosition(SCREEN_WIDTH/3*2, ACTIONBAR_HEIGHT+subViewHeight, SCREEN_WIDTH/3, subViewHeight);

    toolbar.begin();
    toolbar.setButtonLabel(BTN_A, "Detect");
    toolbar.setButtonLabel(BTN_B, "Calib");
    toolbar.setButtonLabel(BTN_C, "LED");
    toolbar.draw();

    prevAccX = 0;
    prevAccY = 0;
    prevAccZ = 0;

    loadFaces();
    // lightUpFaceAll();

}

bool isLEDTest = true;

void loop() {
    // LED検査用
    if (isLEDTest) {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB::Blue;  // すべてのLEDを青色にする
            FastLED.show();
            // delay(1000);
        }
        isLEDTest = false;
        delay(1000);
    }
    else {
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB::Black;  // すべてのLEDを消灯
            FastLED.show();
            // delay(1000);
        }
        isLEDTest = true;
        delay(1000);
    }

    if (actionBar.isBackPressed()) {
        Serial.println("Back button pressed!");
        currentState = STATE_NONE;
        detectState = STATE_DETECTION_INIT;
        calibrationState = STATE_CALIBRATION_INIT;
        ledControlState = STATE_LED_CONTROL_INIT;

        actionBar.setTitle("Main Menu");
        actionBar.setStatus("Ready");
        actionBar.draw();
        mainTextView.setText("System Ready");
        subTextView.setText("");

        toolbar.setButtonLabel(BTN_A, "Detect");
        toolbar.setButtonLabel(BTN_B, "Calib");
        toolbar.setButtonLabel(BTN_C, "LED");
    }

    switch (currentState) {
        case STATE_DETECTION:
            break;
        case STATE_CALIBRATION:
            if (toolbar.getPressedButton(BTN_A)) resetFaces();
            if (toolbar.getPressedButton(BTN_B)) saveFaces();
            if (toolbar.getPressedButton(BTN_C)) loadFaces();
            break;
        case STATE_LED_CONTROL:
            break;
        default:
            // メインメニュー
            if (toolbar.getPressedButton(BTN_A)) changeState(STATE_DETECTION);
            if (toolbar.getPressedButton(BTN_B)) changeState(STATE_CALIBRATION);
            if (toolbar.getPressedButton(BTN_C)) changeState(STATE_LED_CONTROL);
            break;
    }
    processState();

    if (isViewUpdate) {
        mainTextView.draw();
        actionBar.draw();
        toolbar.draw();
        isViewUpdate = false;
    }

    #ifdef _3D_MODEL
    icosahedron.rotate(0.1, 0.12);  // 回転
    icosahedron.draw();  // 描画

    #endif

    delay(200);
}
